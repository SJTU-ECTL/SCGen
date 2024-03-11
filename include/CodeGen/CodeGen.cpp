#include "CodeGen.h"
#include "Modules/SC_core.h"
#include "Nodes/OptimizeNode.h"
#include "Utils/Utils.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

void parse_file(
    const std::string &file_path, const Config &conf,
    std::unordered_map<std::string, Node *> &node_map,
    std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                              std::vector<std::string>>>
        &name_map) {

  ifstream input_file(file_path);

  vector<unique_ptr<Line>> inputs;
  inputs = parse_input_file(input_file);
  vector<Config> configs;
  configs.push_back(conf);
  parse_inputs(inputs, configs, node_map, name_map);
}

void emit_header(ofstream &output_file) {
  output_file << "#include \"Modules/SC_core.h\"" << endl;
  output_file << "#include <iostream>" << endl;
  output_file << endl;
}

void emit_before(ofstream &output_file, const vector<Config> &configs) {
  output_file << "class " << configs[0].graph_name << " : public SC {" << endl;
  output_file << "public:" << endl;
  output_file << "  void init() {" << endl;
  output_file << "    srand(conf.seed);" << endl;
  output_file << endl;
}

void emit_one_circuit(ofstream &output_file, int index, const Config &config) {
  string circuit_var(1, (char)('a' + index));
  string config_var = circuit_var + "_conf";

  output_file << "  Config " << config_var << ";" << endl;
  output_file << config.codegen_str(config_var) << endl;

  output_file << "  " << config.graph_name << " " << circuit_var << ";" << endl;
  output_file << "  " << circuit_var << ".set_conf(" << config_var << ");"
              << endl;
  output_file << "  " << circuit_var
              << ".set_fn([](const std::vector<double> &ins) { " << config.fn
              << " });" << endl;
  // output_file << "  srand(seed);" << endl;
  output_file << "  " << circuit_var << ".init();" << endl;
  // output_file << "  " << circuit_var << ".save_graph(\"" << conf.graph_name
  // << ".output.gv\");"
  //             << endl;
  output_file << "  " << circuit_var << ".simulate();" << endl;
  output_file << endl;
}

void emit_after(ofstream &output_file, const vector<Config> &configs) {

  output_file << "    for (size_t i = 0; i < inputs.size() - 1; i++) {" << endl;
  output_file << "      inputs[i]->set_next_node(inputs[i + 1]);" << endl;
  output_file << "    }" << endl;

  output_file << "    // Generate all_nodes" << endl;
  output_file << "    generate_all_nodes();" << endl;
  output_file << "    optimize();" << endl;
  output_file << "  }" << endl;
  output_file << "};" << endl;

  output_file << "int main() {" << endl;
  // output_file << "    long seed = time(NULL);" << endl;

  int index = 0;
  for (const auto &conf : configs) {
    emit_one_circuit(output_file, index++, conf);
  }

  output_file << "  return 0;" << endl;
  output_file << "}" << endl;
}

// Split the line by space
vector<string> parse_line(string line) {
  // TODO: optimize it later
  vector<string> ans;

  size_t i = 0;
  size_t startIndex = 0, endIndex = 0;

  while (startIndex < line.size() && line[startIndex] == ' ') {
    startIndex++;
  }
  i = startIndex;
  endIndex = startIndex;

  while (i <= line.size()) {
    if (line[i] == ' ' || i == line.size()) {
      endIndex = i;
      string subStr = "";
      subStr.append(line, startIndex, endIndex - startIndex);
      ans.push_back(subStr);
      startIndex = endIndex + 1;
      while (startIndex < line.size() && line[startIndex] == ' ') {
        startIndex++;
      }
      i = startIndex;
    }
    i++;
  }
  return ans;
}

vector<unique_ptr<Line>> parse_input_file(ifstream &input_file) {
  vector<vector<string>> inputs;

  string line;
  vector<string> a_line;

  vector<unique_ptr<Line>> line_arr;

  while (getline(input_file, line)) {
    if (line.empty())
      continue;
    a_line = parse_line(line);

    switch (get_line_type(line)) {
    case DIGRAPH: {
      line_arr.push_back(make_unique<Digraph>(line));
      break;
    }
    case CONFIG: {
      line_arr.push_back(make_unique<Configuration>(line));
      break;
    }
    case DEFINITION: {
      line_arr.push_back(make_unique<Definition>(line));
      break;
    }
    case CONNECTION: {
      line_arr.push_back(make_unique<Connection>(line));
      break;
    }
    case SUBGRAPH_START: {
      line_arr.push_back(make_unique<Subgraph_start>(line));
      break;
    }
    case SUBGRAPH_END: {
      line_arr.push_back(make_unique<Subgraph_end>(line));
      break;
    }
    default: {
      break;
    }
    }

    inputs.push_back(a_line);
  }
  return line_arr;
}

Node *parse_node(
    const Definition *def,
    const unordered_map<string, pair<vector<string>, vector<string>>> &name_map,
    const Config &conf) {

  Node *node = nullptr;
  if (name_map.contains(def->name)) {
    node = NodeFactory::create(
        def->node_type,
        pair<int, int>(name_map.at(def->name).first.size(),
                       name_map.at(def->name).second.size()),
        def->extra, conf);
  } else {
    node = NodeFactory::create(def->node_type, pair<int, int>(0, 0), def->extra,
                               conf);
  }
  assert(node != nullptr);

  return node;
}

int find_in_index(
    const string &in_node, const string &out_node,
    const unordered_map<string, pair<vector<string>, vector<string>>>
        &name_map) {
  vector<string> ins = name_map.at(out_node).first;
  for (size_t i = 0; i < ins.size(); i++) {
    if (ins[i] == in_node) {
      return i;
    }
  }
  return -1;
}

std::string get_name_from_node(const unordered_map<string, Node *> &node_map,
                               const Node *node) {
  for (auto const &[name, node_temp] : node_map) {
    if (node_temp == node)
      return name;
  }
  return "NULL";
}

void parse_definitions(
    const vector<unique_ptr<Line>> &inputs,
    unordered_map<string, pair<vector<string>, vector<string>>> &name_map,
    unordered_map<string, Node *> &node_map, const Config &conf) {
  // string name;
  // unordered_map<string, Node *> node_map;
  for (const auto &in : inputs) {
    if (in->get_type() == DEFINITION) {
      Definition *def = dynamic_cast<Definition *>(in.get());
      Node *new_node = parse_node(def, name_map, conf);
      node_map[def->name] = new_node;
    }
  }

  // build connections
  for (const auto &[name, connections] : name_map) {
    vector<string> outs = connections.second;
    int index = 0;
    for (const auto &out : outs) {
      node_map[name]->connect(index++, node_map[out],
                              find_in_index(name, out, name_map));
    }
  }

  // TODO: temporarily parse subgraph here, move it to other function later.
  int index = 0;
  int length = inputs.size();
  while (index < length) {
    if (inputs[index]->get_type() == SUBGRAPH_START) {
      Subgraph_start *subgraph_start =
          dynamic_cast<Subgraph_start *>(inputs[index].get());
      int input_num = 0;
      int output_num = 0;
      std::vector<Node *> components;
      std::vector<Node *> start_nodes;
      std::vector<Node *> end_nodes;

      index++;
      while (inputs[index]->get_type() != SUBGRAPH_END) {
        assert(inputs[index]->get_type() == CONNECTION);
        Connection *connection =
            dynamic_cast<Connection *>(inputs[index].get());
        components.push_back(node_map[connection->destination]);
        start_nodes.push_back(node_map[connection->source]);
        end_nodes.push_back(node_map[connection->destination]);

        index++;
      }
      auto it = start_nodes.begin();
      while (it != start_nodes.end()) {
        bool found = false;
        for (const auto &component : components) {
          if (*it == component) {
            it = start_nodes.erase(it);
            found = true;
            break;
          }
        }
        if (!found) {
          ++it;
        }
      }
      // TODO: temporarily the output number is set to 1, and is the last item
      // in end_nodes.
      input_num = start_nodes.size();
      output_num = 1;
      end_nodes.erase(end_nodes.begin(), end_nodes.end() - 1);

      SubcircuitNode *subcircuit =
          new SubcircuitNode(input_num, output_num, conf);
      subcircuit->add_components(components);

      node_map[subgraph_start->name] = subcircuit;
      name_map[subgraph_start->name] = pair<vector<string>, vector<string>>(
          vector<string>(input_num, ""), vector<string>(1, ""));

      // TODO: hack node connections to insert subcircuit into the DAG
      int index_input = 0;
      for (const auto start : start_nodes) {
        for (const auto input : start->inputs) {
          int out_index = input->find_out_node(start);
          input->set_out_node(out_index, subcircuit);
          subcircuit->set_in_node(index_input, input);
          std::string input_name = get_name_from_node(node_map, input);
          name_map[input_name].second[out_index] = subgraph_start->name;
          name_map[subgraph_start->name].first[index_input] = input_name;
          index_input++;
        }
      }
      int index_output = 0;
      for (const auto end : end_nodes) {
        for (const auto output : end->outputs) {
          int in_index = output->find_in_node(end);
          output->set_in_node(in_index, subcircuit);
          subcircuit->set_out_node(index_output, output);
          std::string output_name = get_name_from_node(node_map, output);
          name_map[output_name].first[in_index] = subgraph_start->name;
          name_map[subgraph_start->name].second[index_output] = output_name;
          index_output++;
        }
      }
      for (const auto &start : start_nodes) {
        subcircuit->start_nodes_name.push_back(
            get_name_from_node(node_map, start));
      }
      for (const auto &end : end_nodes) {
        subcircuit->end_nodes_name.push_back(get_name_from_node(node_map, end));
      }

      subcircuit->add_inside_circuits(start_nodes, end_nodes);
    }

    index++;
  }
}

void parse_connections(
    const vector<unique_ptr<Line>> &inputs,
    unordered_map<string, pair<vector<string>, vector<string>>> &name_map) {
  for (const auto &a_line : inputs) {
    if (a_line->get_type() == CONNECTION) {
      Connection *connection = dynamic_cast<Connection *>(a_line.get());
      if (!name_map.contains(connection->source)) {
        name_map[connection->source] = pair<vector<string>, vector<string>>();
      }
      if (!name_map.contains(connection->destination)) {
        name_map[connection->destination] =
            pair<vector<string>, vector<string>>();
      }
      name_map[connection->source].second.push_back(connection->destination);
      name_map[connection->destination].first.push_back(connection->source);
    }
  }
}

/**
 * @brief Set the config Object
 *  If the config only has one value, ignore the index variable.
 *  If the config has multi-values, set corresponding value according to index.
 *
 * @param conf
 * @param index
 */
void set_config(Config &conf, const Configuration *configuration, int index) {
  string value;
  if (configuration->mul_value.empty()) {
    value = configuration->value;
  } else {
    value = configuration->mul_value[index];
  }

  conf.set_value(configuration->name, value);
}

void config_helper(const Configuration *configuration,
                   vector<Config> &configs) {
  if (configuration->mul_value.empty()) {
    for (auto &conf : configs) {
      set_config(conf, configuration, 0);
    }
    return;
  }

  int num = configuration->mul_value.size();
  vector<Config> new_configs;
  for (int index = 0; index < num; index++) {
    for (const auto &conf : configs) {
      Config new_conf = conf;
      set_config(new_conf, configuration, index);
      new_configs.push_back(new_conf);
    }
  }
  configs = new_configs;
}

void parse_inputs(
    const vector<unique_ptr<Line>> &inputs, vector<Config> &configs,
    unordered_map<string, Node *> &node_map,
    unordered_map<string, pair<vector<string>, vector<string>>> &name_map) {
  // parse config
  vector<Configuration *> configurations;
  for (auto &input : inputs) {
    if (input->get_type() == CONFIG) {
      configurations.push_back(dynamic_cast<Configuration *>(input.get()));
    }
  }

  if (configs.empty()) {
    configs.push_back(Config());
    // Reverse traversal to keep the configuration at front is gathered together
    for (int index = configurations.size() - 1; index >= 0; index--) {
      config_helper(configurations[index], configs);
    }

    // parse graph name
    for (auto &input : inputs) {
      if (input->get_type() == DIGRAPH) {
        for (auto &conf : configs)
          conf.graph_name = dynamic_cast<Digraph *>(input.get())->graph_name;
        break;
      }
    }
  }

  // parse DAG
  parse_connections(inputs, name_map);
  // Using an arbitrary config seems fine here.
  parse_definitions(inputs, name_map, node_map, configs[0]);
}

// ========================================
// |     IMPLEMENTATION OF CODEGEN.H      |
// ========================================

Line_type get_line_type(std::string line) {
  trim(line);

  if (!line.starts_with("//") && line.find("->") != string::npos) {
    return CONNECTION;
  } else if (line.starts_with("/*")) {
    return DEFINITION;
  } else if (line.starts_with("digraph")) {
    return DIGRAPH;
  } else if (line.starts_with("// Config:")) {
    return CONFIG;
  } else if (line.starts_with("subgraph")) {
    return SUBGRAPH_START;
  } else if (line.starts_with("}")) {
    return SUBGRAPH_END;
  }
  return GV_COMMAND;
}

void Line::lex_space(std::string &line) {
  if (line.size() == 0)
    return;
  int end = 0;
  int len = line.size();
  while (end < len && line[end] == ' ') {
    end++;
  }
  line = line.substr(end, line.size() - end);
}

std::string Line::lex(std::string &line) {
  if (line.size() == 0)
    return "";

  lex_space(line);
  int end = 0;
  int len = line.size();

  bool is_token = false;
  if (tokens.contains(line[end])) {
    end++;
    is_token = true;
  }

  if (!is_token) {
    for (const auto &tok : tokens2) {
      if (line.starts_with(tok)) {
        end += 2;
        is_token = true;
        break;
      }
    }
  }

  // Parse double quote
  if (!is_token) {
    if (line[end] == '"') {
      do {
        end++;
      } while (line[end] != '"');
      end++;
      is_token = true;
    }
  }

  if (!is_token) {
    while (end < len) {
      if (tokens.contains(line[end])) {
        break;
      }
      for (const auto &tok : tokens2) {
        if (line.starts_with(tok)) {
          break;
        }
      }
      end++;
    }
  }
  std::string ans = line.substr(0, end);
  line = line.substr(end, len - end);
  return ans;
}

std::vector<std::string> Line::parse(std::string line) {
  std::vector<std::string> arr;
  std::string temp;
  do {

    temp = lex(line);
    arr.push_back(temp);

  } while (temp != "");

  if (arr.back() == "") {
    arr.pop_back();
  }

  return arr;
}

Configuration::Configuration(std::string line) {
  std::vector<std::string> arr = parse(line);
  assert(arr[0] == "//" && arr[1] == "Config:");
  name = arr[2];

  // for fn config
  if (arr[2] == "fn") {
    for (size_t index = 3; index < arr.size(); index++) {
      value += " " + arr[index];
    }
    return;
  }

  // for multi config
  if (arr[3] == "[") {
    value = "multi-value";
    int index = 4;
    while (arr[index] != "]") {
      if (arr[index] != ",")
        mul_value.push_back(arr[index]);
      index++;
    }

    // if only have one multi-value, just move it to value.
    if (mul_value.size() == 1) {
      value = mul_value[0];
      mul_value.clear();
    }
  } else {
    value = arr[3];
    mul_value = std::vector<std::string>();
  }
}

Connection::Connection(std::string line) {
  std::vector<std::string> arr = parse(line);
  assert(arr[1] == "->");
  source = arr[0];
  destination = arr[2];
  if (arr[3] == "[") {
    size_t index = 4;
    while (index < arr.size() && arr[index] != "]") {
      std::pair<std::string, std::string> attr;
      attr.first = arr[index];
      assert(arr[index + 1] == "=");
      attr.second = arr[index + 2];

      attributes.push_back(attr);

      index += 3;
      if (arr[index] == ",")
        index++;
    }
  }
}

Digraph::Digraph(std::string line) {
  std::vector<std::string> arr = parse(line);
  assert(arr[0] == "digraph");
  graph_name = arr[1];
}

Subgraph_start::Subgraph_start(std::string line) {
  std::vector<std::string> arr = parse(line);
  assert(arr[0] == "subgraph");
  name = arr[1];
}

Subgraph_end::Subgraph_end(std::string line) {
  std::vector<std::string> arr = parse(line);
  assert(arr[0] == "}");
}

Definition::Definition(std::string line) {
  std::vector<std::string> arr = parse(line);
  assert(arr[0] == "/*" && arr[2] == "*/");
  node_type = arr[1];
  name = arr[3];
  size_t index = 5;
  if (arr[4] == "[") {
    while (index < arr.size() && arr[index] != "]") {
      std::pair<std::string, std::string> attr;
      attr.first = arr[index];
      assert(arr[index + 1] == "=");
      attr.second = arr[index + 2];

      attributes.push_back(attr);

      index += 3;
      if (arr[index] == ",")
        index++;
    }
  }

  // Parse extra infomation
  while (index < arr.size() && arr[index] != "//") {
    index++;
  }

  if (index == arr.size())
    return;

  index++;
  while (index < arr.size()) {
    extra.push_back(arr[index]);
    index++;
  }
}