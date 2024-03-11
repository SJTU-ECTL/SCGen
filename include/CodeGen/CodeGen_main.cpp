#include "CodeGen.h"
#include "Nodes/OptimizeNode.h"
#include <unordered_map>

using namespace std;

int main(int argc, char *argv[]) {
  cout << "Generating cpp file..." << endl;
  string file_name = argv[1];
  cout << "\tInput file: " << file_name << endl;

  string output_file_name = argv[2];
  cout << "\tOutput file: " << output_file_name << endl;

  ifstream input_file(file_name);
  ofstream output_file(output_file_name);

  vector<unique_ptr<Line>> inputs;
  inputs = parse_input_file(input_file);
  // Config conf;
  vector<Config> configs;
  unordered_map<string, Node *> node_map;
  unordered_map<string, pair<vector<string>, vector<string>>> name_map;
  parse_inputs(inputs, configs, node_map, name_map);
  assert(!configs.empty());

  cout << "\tGraph Name: " << configs[0].graph_name << endl;

  emit_header(output_file);
  emit_before(output_file, configs);

  // Store subcircuits nodes, and emit them at last
  std::unordered_map<std::string, Node *> subcircuits;

  // temp here: Try to emit
  for (const auto &[name, node] : node_map) {
    if (dynamic_cast<SubcircuitNode *>(node)) {
      subcircuits[name] = node;
      continue;
    }
    std::string codegen_str = std::regex_replace(node_map[name]->codegen_str(),
                                                 std::regex("<-name->"), name);
    size_t str_find = codegen_str.find(name + "_extra");
    std::vector<std::string> extra = node->get_extra();
    if (str_find != std::string::npos) {
      // Emit extra information
      output_file << "    std::vector<std::string> " + name + "_extra({";
      if (!extra.empty()) {
        for (int i = 0; i < extra.size() - 1; i++) {
          output_file << "\"" << extra[i] << "\", ";
        }
        output_file << "\"" << extra[extra.size() - 1] << "\"";
      }
      output_file << "});" << std::endl;
    }

    output_file << codegen_str << endl;
    if (extra.size() > 3 && extra[0] == "True") {
      output_file << "*" << name << "=*" << extra[2] << ";" << std::endl;
    }
    output_file << endl;
  }

  // Emit subcircuit
  std::vector<string> strings_after;
  for (const auto &[name, node] : subcircuits) {
    std::string codegen_str = std::regex_replace(node_map[name]->codegen_str(),
                                                 std::regex("<-name->"), name);
    size_t str_find = codegen_str.find(name + "_extra");
    std::vector<std::string> extra = node->get_extra();
    if (str_find != std::string::npos) {
      // Emit extra information
      output_file << "    std::vector<std::string> " + name + "_extra({";
      if (!extra.empty()) {
        for (int i = 0; i < extra.size() - 1; i++) {
          output_file << "\"" << extra[i] << "\", ";
        }
        output_file << "\"" << extra[extra.size() - 1] << "\"";
      }
      output_file << "});" << std::endl;
    }

    output_file << codegen_str << endl;
    if (extra.size() > 3 && extra[0] == "True") {
      output_file << "*" << name << "=*" << extra[2] << ";" << std::endl;
    }
    output_file << endl;

    strings_after.push_back(std::regex_replace(
        dynamic_cast<SubcircuitNode *>(node)->codegen_str_after(),
        std::regex("<-name->"), name));
  }

  // temp here: try to emit connect
  for (const auto &[name, node] : node_map) {
    int index = 0;
    for (const auto &out : name_map[name].second) {
      output_file << "    " << name << "->connect(" << std::to_string(index)
                  << ", " << out << ", " << find_in_index(name, out, name_map)
                  << ");" << endl;
      index++;
    }
  }
  output_file << endl;

  for (const auto &str : strings_after) {
    output_file << str;
  }
  output_file << std::endl;

  emit_after(output_file, configs);

  input_file.close();
  output_file.close();

  cout << "Generating cpp file complete!" << endl;
  cout << endl;
}