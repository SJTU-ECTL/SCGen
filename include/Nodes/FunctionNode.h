/**
 * @brief
 *     The definiton and implementation of functional nodes, such as AndNode and
 *         OrNode.
 */

#ifndef FUNCTIONNODE_H
#define FUNCTIONNODE_H

#include "Node.h"
#include "Utils/Utils.h"
#include "Utils/Utils_IO.h"
#include <cassert>
#include <sstream>

// An virtual class used to represent function nodes.
class FunctionNode : public Node {
public:
  FunctionNode(int _input_num, int _output_num, const Config &_conf)
      : Node(_input_num, _output_num, _conf) {}

  virtual void simple_run(){};
  virtual Node *copy() = 0;
};

class NNode : public FunctionNode {
public:
  NNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {}

  std::string label() { return "Not"; }
  std::string codegen_str() {
    return "    NNode *<-name-> = new NNode(" + std::to_string(input_num) +
           ", " + std::to_string(output_num) + ", conf" + ");";
  }
  virtual Node *copy() {
    Node *node = new NNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] ^= 1;
    }
#if LOG
    std::cout << "NNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;

    if (conf.optimize & INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      for (const auto &up : update_temp) {
        int temp = inputs[0]->data[up] ^ 1;
        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        }
      }
    } else {
      data = std::vector<int>(inputs[0]->data);
      for (size_t index = 0; index < data.size(); ++index) {
        data[index] ^= 1;
      }
    }

#if LOG
    std::cout << "NNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] ^= 1;
    }

    ready = true;
    notify_simple();
  }
};

class XorNode : public FunctionNode {
public:
  XorNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {}

  std::string label() { return "Xor"; }
  std::string codegen_str() {
    return "    XorNode *<-name-> = new XorNode(" + std::to_string(input_num) +
           ", " + std::to_string(output_num) + ", conf" + ");";
  }

  virtual Node *copy() {
    Node *node = new XorNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    // Assume that Xor, And, Nand, Or, Add will not connect to non-verilog node
    // such as Calculate node
    std::string input_str = "\t\t{";
    size_t input_index = 0;
    for (; input_index < inputs.size() - 1; input_index++) {
      if (node_index_map.contains(
              inputs[input_index])) // ensure Node such as CalculateNode is not
                                    // considered
        input_str += "wire_" + std::to_string(inputs[input_index]->id) + "_" +
                     std::to_string(this->id) + ",";
    }
    if (node_index_map.contains(
            inputs[input_index])) // ensure Node such as CalculateNode is not
                                  // considered
      input_str += "wire_" + std::to_string(inputs[input_index]->id) + "_" +
                   std::to_string(this->id) + "},\n";

    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (node_index_map.contains(i)) {
        input_str += "\t\twire_" + std::to_string(this->id) + "_" +
                     std::to_string(i->id);
        break;
      }
    }
    return input_str;
  }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "\t" + label() + " #(.bit_width(bit_width),.input_num(" +
           std::to_string(this->inputs.size()) + ")) " + label() + "_" +
           std::to_string(this->id) + "(\n" +
           verilog_codegen_io_str(node_index_map) + ");\n";
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (size_t input_index = 1; input_index < inputs.size(); input_index++) {
      for (size_t index = 0; index < inputs[input_index]->data.size();
           ++index) {
        data[index] = data[index] ^ inputs[input_index]->data[index];
      }
    }
#if LOG
    std::cout << "AndNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize == NONE || conf.optimize == LOOKUPTABLE) {
      data = std::vector<int>(inputs[0]->data);
      for (size_t input_index = 1; input_index < inputs.size(); input_index++) {
        for (size_t index = 0; index < inputs[input_index]->data.size();
             ++index) {
          data[index] = data[index] ^ inputs[input_index]->data[index];
        }
      }
    } else if (conf.optimize == INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      for (const auto &up : update_temp) {
        // TODO
        int temp = inputs[0]->data[up] & inputs[1]->data[up];
        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        } else {
        }
      }
    }
#if LOG
    std::cout << "XorNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t input_index = 1; input_index < inputs.size(); input_index++) {
      for (size_t index = 0; index < inputs[input_index]->data.size();
           ++index) {
        data[index] = data[index] ^ inputs[input_index]->data[index];
      }
    }
    ready = true;
    notify_simple();
  }
};

class XnorNode : public FunctionNode {
public:
  XnorNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {}

  std::string label() { return "Xnor"; }
  std::string codegen_str() {
    return "    XnorNode *<-name-> = new XnorNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");";
  }

  std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    // Assume that Xnor, Xor, And, Nand, Or, Add will not connect to non-verilog
    // node such as Calculate node
    std::string input_str = "\t\t{";
    size_t input_index = 0;
    for (; input_index < inputs.size() - 1; input_index++) {
      if (node_index_map.contains(
              inputs[input_index])) // ensure Node such as CalculateNode is not
                                    // considered
        input_str += "wire_" +
                     std::to_string(node_index_map[inputs[input_index]]) + "_" +
                     std::to_string(node_index_map[this]) + ",";
    }
    if (node_index_map.contains(
            inputs[input_index])) // ensure Node such as CalculateNode is not
                                  // considered
      input_str += "wire_" +
                   std::to_string(node_index_map[inputs[input_index]]) + "_" +
                   std::to_string(node_index_map[this]) + "},\n";

    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (node_index_map.contains(i)) {
        input_str += "\t\twire_" + std::to_string(node_index_map[this]) + "_" +
                     std::to_string(node_index_map[i]);
        break;
      }
    }
    return input_str;
  }

  std::string verilog_module_init_str();

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    // return "\t" + label() + " #(.bit_width(bit_width),.input_num(" +
    //        std::to_string(this->inputs.size()) + ")) " + label() + "_" +
    //        std::to_string(node_index_map[this]) + "(\n" +
    //        verilog_codegen_io_str(node_index_map) + ");\n";
    return "";
  }
  virtual Node *copy() {
    Node *node = new XnorNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    // Xor
    for (size_t input_index = 1; input_index < inputs.size(); input_index++) {
      for (size_t index = 0; index < inputs[input_index]->data.size();
           ++index) {
        data[index] = data[index] ^ inputs[input_index]->data[index];
      }
    }
    // Not
    for (int &index : data) {
      index = !index;
    }
#if LOG
    std::cout << "AndNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize & INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      for (const auto &up : update_temp) {
        int temp = inputs[0]->data[up] ^ inputs[1]->data[up];
        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        }
      }
    } else {
      data = std::vector<int>(inputs[0]->data);
      for (size_t input_index = 1; input_index < inputs.size(); input_index++) {
        for (size_t index = 0; index < inputs[input_index]->data.size();
             ++index) {
          data[index] = data[index] ^ inputs[input_index]->data[index];
        }
      }
      // Not
      for (int &index : data) {
        index = !index;
      }
    }
#if LOG
    std::cout << "XorNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t input_index = 1; input_index < inputs.size(); input_index++) {
      for (size_t index = 0; index < inputs[input_index]->data.size();
           ++index) {
        data[index] = data[index] ^ inputs[input_index]->data[index];
      }
    }
    // Not
    for (int &index : data) {
      index = !index;
    }

    ready = true;
    notify_simple();
  }
};

class AndNode : public FunctionNode {
public:
  AndNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {}

  std::string label() { return "And"; }
  std::string codegen_str() {
    return "    AndNode *<-name-> = new AndNode(" + std::to_string(input_num) +
           ", " + std::to_string(output_num) + ", conf" + ");";
  }

  std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    std::stringstream str;

    // Inputs
    str << "\t\t{";
    for (int i = 0; i < inputs.size() - 1; i++) {
      str << verilog_wire_name(inputs[i], this) + ", ";
    }
    str << verilog_wire_name(inputs[inputs.size() - 1], this);
    str << "},\n";

    // Output
    // definition of Verilog Module only contain one output
    // the connection of this output to different node is handled in
    // verilog_codegen_multi_output
    str << "\t\t" << verilog_wire_name(this, outputs[0]) + "\n";

    return str.str();
  }

  std::string verilog_module_init_str() {
    std::stringstream str;
    str << "\t" << label() << " #(.bit_width(bit_width),.input_num("
        << this->inputs.size() << ")) " << label() << "_" << this->id << "(\n"
        << verilog_codegen_io_str({}) << "\t);\n";
    return str.str();
  };

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "";
  }

  virtual Node *copy() {
    Node *node = new AndNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (const auto &i : inputs) {
      for (size_t index = 0; index < i->data.size(); ++index) {
        data[index] = data[index] & i->data[index];
      }
    }
#if LOG
    std::cout << "AndNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;

    if (conf.optimize & INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      for (const auto &up : update_temp) {
        int temp = 1;
        for (const auto &i : inputs) {
          temp &= i->data[up];
        }
        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        }
      }
    } else {
      data = std::vector<int>(inputs[0]->data);
      for (const auto &i : inputs) {
        for (size_t index = 0; index < i->data.size(); ++index) {
          data[index] = data[index] & i->data[index];
        }
      }
    }
#if LOG
    std::cout << "AndNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (const auto &i : inputs) {
      for (size_t index = 0; index < i->data.size(); ++index) {
        data[index] = data[index] & i->data[index];
      }
    }

    ready = true;
    notify_simple();
  }
};

class OrNode : public FunctionNode {
public:
  OrNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {}

  std::string label() { return "Or"; }
  std::string codegen_str() {
    return "    OrNode *<-name-> = new OrNode(" + std::to_string(input_num) +
           ", " + std::to_string(output_num) + ", conf" + ");";
  }

  std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    std::string input_str = "\t\t{";
    size_t input_index = 0;
    for (; input_index < inputs.size() - 1; input_index++) {
      if (node_index_map.contains(
              inputs[input_index])) // ensure Node such as CalculateNode is not
                                    // considered
        input_str += "wire_" +
                     std::to_string(node_index_map[inputs[input_index]]) + "_" +
                     std::to_string(node_index_map[this]) + ",";
    }
    if (node_index_map.contains(
            inputs[input_index])) // ensure Node such as CalculateNode is not
                                  // considered
      input_str += "wire_" +
                   std::to_string(node_index_map[inputs[input_index]]) + "_" +
                   std::to_string(node_index_map[this]) + "},\n";

    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (node_index_map.contains(i)) {
        input_str += "\t\twire_" + std::to_string(node_index_map[this]) + "_" +
                     std::to_string(node_index_map[i]);
        break;
      }
    }
    return input_str;
  }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "\t" + label() + " #(.bit_width(bit_width),.input_num(" +
           std::to_string(this->inputs.size()) + ")) " + label() + "_" +
           std::to_string(node_index_map[this]) + "(\n" +
           verilog_codegen_io_str(node_index_map) + ");\n";
  }

  virtual Node *copy() {
    Node *node = new OrNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  void reset_zero() {
    // TODO
    assert(0 && "Not implemented");
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (const auto &i : inputs) {
      for (size_t index = 0; index < i->data.size(); ++index) {
        data[index] = data[index] & i->data[index];
      }
    }
#if LOG
    std::cout << "AndNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize & INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      for (const auto &up : update_temp) {
        int temp = 1;
        for (const auto &i : inputs) {
          temp &= i->data[up];
        }
        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        } else {
        }
      }
    } else {
      data = std::vector<int>(inputs[0]->data);
      for (const auto &i : inputs) {
        for (size_t index = 0; index < i->data.size(); ++index) {
          data[index] = data[index] | i->data[index];
        }
      }
    }
#if LOG
    std::cout << "AndNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    assert(0 && "Not implemented");
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (const auto &i : inputs) {
      for (size_t index = 0; index < i->data.size(); ++index) {
        data[index] = data[index] & i->data[index];
      }
    }

    ready = true;
    notify_simple();
  }
};

class NandNode : public FunctionNode {
public:
  NandNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {}

  std::string label() { return "Nand"; }
  std::string codegen_str() {
    return "    NandNode *<-name-> = new NandNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");";
  }

  std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    std::string input_str = "\t\t{";
    size_t input_index = 0;
    for (; input_index < inputs.size() - 1; input_index++) {
      if (node_index_map.contains(
              inputs[input_index])) // ensure Node such as CalculateNode is not
                                    // considered
        input_str += "wire_" +
                     std::to_string(node_index_map[inputs[input_index]]) + "_" +
                     std::to_string(node_index_map[this]) + ",";
    }
    if (node_index_map.contains(
            inputs[input_index])) // ensure Node such as CalculateNode is not
                                  // considered
      input_str += "wire_" +
                   std::to_string(node_index_map[inputs[input_index]]) + "_" +
                   std::to_string(node_index_map[this]) + "},\n";

    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (node_index_map.contains(i)) {
        input_str += "\t\twire_" + std::to_string(node_index_map[this]) + "_" +
                     std::to_string(node_index_map[i]);
        break;
      }
    }
    return input_str;
  }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "\t" + label() + " #(.bit_width(bit_width),.input_num(" +
           std::to_string(this->inputs.size()) + ")) " + label() + "_" +
           std::to_string(node_index_map[this]) + "(\n" +
           verilog_codegen_io_str(node_index_map) + ");\n";
  }

  virtual Node *copy() {
    Node *node = new NandNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (const auto &i : inputs) {
      for (size_t index = 0; index < i->data.size(); ++index) {
        data[index] = data[index] & i->data[index];
      }
    }
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] ^= 1;
    }
#if LOG
    std::cout << "NandNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize & INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      for (const auto &up : update_temp) {
        int temp = 1;
        for (const auto &i : inputs) {
          temp &= i->data[up];
        }
        temp ^= 1;
        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        }
      }
    } else {
      data = std::vector<int>(inputs[0]->data);
      for (const auto &i : inputs) {
        for (size_t index = 0; index < i->data.size(); ++index) {
          data[index] = data[index] & i->data[index];
        }
      }
      for (size_t index = 0; index < data.size(); ++index) {
        data[index] ^= 1;
      }
    }
#if LOG
    std::cout << "NandNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (const auto &i : inputs) {
      for (size_t index = 0; index < i->data.size(); ++index) {
        data[index] = data[index] & i->data[index];
      }
    }
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] ^= 1;
    }

    ready = true;
    notify_simple();
  }
};

class DFFNode : public Node {
private:
  int DFF;

public:
  DFFNode(int _input_num, int _output_num, const Config &_conf)
      : Node(_input_num, _output_num, _conf), DFF(1) {
    assert(_input_num == 1);
  }
  void init(const std::vector<std::string> &extra) {
    DFF = std::stoi(extra[0]);
  }

  DFFNode(int _input_num, int _output_num, const Config &_conf, int _DFF)
      : Node(_input_num, _output_num, _conf), DFF(_DFF) {
    assert(_input_num == 1);
  }

  virtual Node *copy() {
    Node *new_node = new DFFNode(this->input_num, this->output_num, this->conf);
    new_node->init(this->extra);
    new_node->id = this->id;
    return new_node;
  }

  std::string label() { return "DFF"; }
  std::string codegen_str() {
    return "    DFFNode *<-name-> = new DFFNode(" + std::to_string(input_num) +
           ", " + std::to_string(output_num) + ", conf, " +
           std::to_string(DFF) + ");";
  }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "\t" + label() + " #(.bit_width(bit_width)) " + label() + "_" +
           std::to_string(node_index_map[this]) + "(\n" +
           verilog_codegen_io_str(node_index_map) + "\t\t,clk);\n";
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); index++) {
      data[(index + DFF) % data.size()] = inputs[0]->data[index];
    }
#if LOG
    std::cout << "DFFNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize & INCREMENTAL) {
      update.clear();
      for (const auto &up : inputs[0]->update) {
        int index = (up + DFF) % data.size();
        data[index] = inputs[0]->data[up];
        update.insert(index);
      }
    } else {
      data = std::vector<int>(inputs[0]->data);
      for (size_t index = 0; index < data.size(); index++) {
        data[(index + DFF) % data.size()] = inputs[0]->data[index];
      }
    }
#if LOG
    std::cout << "DFFNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); index++) {
      data[(index + DFF) % data.size()] = inputs[0]->data[index];
    }

    ready = true;
    notify_simple();
  }
};

class AddNode : public FunctionNode {
  // the last input will always be the select signal

public:
  AddNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {}

  std::string label() { return "Add"; }
  std::string codegen_str() {
    return "    AddNode *<-name-> = new AddNode(" + std::to_string(input_num) +
           ", " + std::to_string(output_num) + ", conf" + ");";
  }

  std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    std::stringstream str;

    // Inputs
    str << "\t\t{";
    for (int i = 0; i < inputs.size() - 2; i++) {
      str << verilog_wire_name(inputs[i], this) + ", ";
    }
    str << verilog_wire_name(inputs[inputs.size() - 2], this);
    str << "},\n";

    // Selection signal
    str << "\t\t" << verilog_wire_name(inputs[inputs.size() - 1], this)
        << ",\n";

    // Output
    // definition of Verilog Module only contain one output
    // the connection of this output to different node is handled in
    // verilog_codegen_multi_output
    str << "\t\t" << verilog_wire_name(this, outputs[0]) + "\n";

    return str.str();
  }

  std::string verilog_module_init_str() {
    std::stringstream str;
    str << "\t" << label() << " #(.bit_width(bit_width),.input_num("
        << this->inputs.size() - 1 << "),.select_bit_width("
        << int(log(this->inputs.size())) << ")) " << label() << "_" << this->id
        << "(\n"
        << verilog_codegen_io_str({}) << "\t);\n";
    return str.str();
  };

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "";
  }
  virtual Node *copy() {
    Node *node = new AddNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    const std::vector<int> &constant = inputs.back()->data;
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] = inputs[constant[index]]->data[index];
    }
#if LOG
    std::cout << "AddNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize & INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      const std::vector<int> &constant = inputs.back()->data;
      for (const auto &up : update_temp) {
        int temp = inputs[constant[up]]->data[up];

        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        }
      }
    } else {   // LOOKUP_TABLE or RUNNING_MINIMAL or None optimization
      data = std::vector<int>(inputs[0]->data);
      const std::vector<int> &constant = inputs.back()->data;
      for (size_t index = 0; index < constant.size(); ++index) {
        data[index] = inputs[constant[index]]->data[index];
      }
    }
#if LOG
    std::cout << "AddNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    const std::vector<int> &constant = inputs.back()->data;
    for (size_t index = 0; index < constant.size(); ++index) {
      data[index] = inputs[constant[index]]->data[index];
    }

    ready = true;
    notify_simple();
  }
};

// FIXME: implement optimization methods for all newly added nodes.
class FSMXKNode : public FunctionNode {
private:
  int state; // 0-7 state
  int get_output(int X, int K) {
    if (X == 0 && K == 0) {
      if (state == 0 || state == 4) {
        return state;
      }
      state--;
      return state;
    }
    if (X == 0 && K == 1) {
      if (state < 4) {
        return state;
      }
      state -= 4;
      return state;
    }
    if (X == 1 && K == 0) {
      if (state > 3) {
        return state;
      }
      state += 4;
      return state;
    }
    if (X == 1 && K == 1) {
      if (state == 3 || state == 7) {
        return state;
      }
      state++;
      return state;
    }
  }

public:
  FSMXKNode(int _input_num, int _output_num, const Config &_conf)
      : FunctionNode(_input_num, _output_num, _conf) {
    state = 0;
  }

  std::string label() { return "FSMXK"; }
  std::string codegen_str() {
    return "    FSMXKNode *<-name-> = new FSMXKNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");";
  }

  virtual Node *copy() {
    FSMXKNode *node =
        new FSMXKNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    node->state = this->state;
    return node;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] = get_output(inputs[0]->data[index], inputs[1]->data[index]);
    }
#if LOG
    std::cout << "FSMXKNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize == NONE) {
      data = std::vector<int>(inputs[0]->data);
      for (size_t index = 0; index < data.size(); ++index) {
        data[index] =
            get_output(inputs[0]->data[index], inputs[1]->data[index]);
      }
    } else {
      // Optimization unimplemented
      exit(-1);
    }
#if LOG
    std::cout << "FSMXKNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] = get_output(inputs[0]->data[index], inputs[1]->data[index]);
    }

    ready = true;
    notify_simple();
  }
};

class SCTanhNode : public FunctionNode {
  // Reference: "Stochastic Neural Computation I: Computational Elements",
  // Bradley D. Brown

  // FSM for tanh N/2 x
private:
  int state; // 4 state
  int N;     // num of states
  int get_output(int X) {
    switch (X) {
    case 0:
      if (state != 0)
        state--;
      break;
    case 1:
      if (state != N - 1)
        state++;
      break;
    default:
      break;
    }
    if (state <= N / 2 - 1) {
      return 0;
    } else {
      return 1;
    }
  }

public:
  SCTanhNode(int _input_num, int _output_num, const Config &_conf, int _N = 16)
      : FunctionNode(_input_num, _output_num, _conf) {
    N = _N;
    state = N / 2 - 1;
  }

  std::string label() { return "SCTanh"; }
  std::string codegen_str() {
    return "    SCTanhNode *<-name-> = new SCTanhNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");";
  }

  std::string verilog_module_init_str();

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "";
  };

  virtual Node *copy() {
    SCTanhNode *node =
        new SCTanhNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    node->N = this->N;
    node->state = this->state;
    return node;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] = get_output(inputs[0]->data[index]);
    }
    state = N / 2 - 1; // reset state
#if LOG
    std::cout << "FSMXNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready()) {
      return;
    }
    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] = get_output(inputs[0]->data[index]);
      // std::cout<<"state "<<state<<" output "<<data[index]<<std::endl;
    }
    // state = N/2-1;
#if LOG
    std::cout << "SCTanhNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }

  void simple_run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);
    for (size_t index = 0; index < data.size(); ++index) {
      data[index] = get_output(inputs[0]->data[index]);
    }

    ready = true;
    notify_simple();
  }
};

#endif
