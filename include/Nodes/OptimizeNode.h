/**
 * @brief
 *     The definiton and implementation of OptimizeNode, such as
 *         LookupTableNode.
 *     Used to optimize the simulation speed.
 */

#ifndef OPTIMIZENODE_H
#define OPTIMIZENODE_H

#include "Node.h"
#include "Nodes/FunctionNode.h"
#include <cassert>
#include <cmath>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class LookupTableNode : public Node {
public:
  std::vector<FunctionNode *> starts;
  std::vector<FunctionNode *> ends;

private:
  std::unordered_map<int, int> lookup_table;

  // class used to generate lookup table
  class DataNode : public Node {
  protected:
    DataNode *next = nullptr;

  public:
    DataNode(int _input_num, int _output_num, const Config &_conf)
        : Node(_input_num, _output_num, _conf) {}

    void set_next_node(DataNode *_next) { next = _next; }

    virtual Node *copy() {
      assert(0 && "Not implemented");
      return nullptr;
    }

    std::string label() { return "LookupTableNode::DataNode"; }
    std::string codegen_str() {
      assert(0 && "Not implemented");
      return "";
    }

    bool verilog_has_instance() override { return false; }

    std::string
    verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
      return "";
    }
    std::string verilog_codegen_multi_output(
        std::unordered_map<Node *, int> node_index_map) {
      return "";
    }
    void set_data(std::vector<int> _data) { data = _data; }

    void reset_zero() {
      assert(0 && "Not implemented");
      clear_ready();
      data = std::vector<int>(1, 0);

#if LOG
      std::cout << "-----"
                << "SourceNode reset_zero(): " << data[0] << "-----"
                << std::endl;
#endif

      ready = true;
      notify_reset_zero();
      // it should not call next->reset_zero(), because the next source node
      // will do it itself in its run() function.
    };

    void run() {
      clear_ready();
#if LOG
      std::cout << "-----"
                << "LookupTableNode::DataNode:";
      print_vector(std::cout, data);
      std::cout << "-----" << std::endl;
#endif
      ready = true;
      notify_simple();
      // Recursively assign all DataNodes
      if (next != nullptr) {
        next->run();
      }
      clear_ready();
    }
  };

public:
  LookupTableNode(int _input_num, int _output_num, const Config &_conf)
      : Node(_input_num, _output_num, _conf) {}

  std::string label() { return "LOOKUPTABLE"; }
  std::string codegen_str() {
    // return "    AndNode *<-name-> = new AndNode(" + std::to_string(input_num)
    // +
    //        ", " + std::to_string(output_num) + ", conf" + ");";
    return "";
  }

  bool verilog_has_instance() override { return false; }

  virtual Node *copy() {
    assert(0 && "Not implemented");
    return nullptr;
  }

  void add_inside_circuits(std::vector<FunctionNode *> _starts,
                           std::vector<FunctionNode *> _ends) {
    starts = _starts;
    ends = _ends;
    // build lookup table
    std::vector<std::vector<int>> combinations(input_num, std::vector<int>());
    for (int n = 0; n < pow(2, input_num); n++) {
      int temp = n;
      for (auto &in : combinations) {
        in.push_back(temp & 1);
        temp >>= 1;
      }
    }

    std::vector<DataNode *> input_nodes;
    int index = 0;
    for (const auto &start : starts) {
      for (int i = 0; i < start->input_num; i++) {
        if (!dynamic_cast<FunctionNode *>(start->inputs[i])) {
          input_nodes.push_back(new DataNode(0, 1, conf));
          input_nodes[index]->connect(0, start, i);
          index++;
        }
      }
    }
    for (int i = 0; i < input_num; i++) {
      if (i < input_num - 1)
        input_nodes[i]->set_next_node(input_nodes[i + 1]);

      input_nodes[i]->set_data(combinations[i]);
    }

    // Simulate to get output data
    input_nodes[0]->run();
    std::vector<int> data = ends[0]->data;

#if LOG
    std::cout << "-----"
              << "LookupTable        result:";
    print_vector(std::cout, data);
    std::cout << "-----" << std::endl;
#endif

    for (size_t n = 0; n < data.size(); n++) {
      lookup_table[n] = data[n];
    }
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (size_t i = 0; i < data.size(); i++) {
      int now = 0;
      for (int j = input_num - 1; j >= 0; j--) {
        now = (now << 1) + inputs[j]->data[i];
      }
      data[i] = lookup_table[now];
    }
#if LOG
    std::cout << "LookupTableNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    assert((conf.optimize & LOOKUPTABLE) &&
           "Only the optimize 2 will have this "
           "LookupTableNode");

    if (!(conf.optimize & INCREMENTAL)) {
      data = std::vector<int>(inputs[0]->data.size());

      for (size_t i = 0; i < data.size(); i++) {
        int now = 0;
        for (int j = input_num - 1; j >= 0; j--) {
          now = (now << 1) + inputs[j]->data[i];
        }
        data[i] = lookup_table[now];
      }
    } else if (conf.optimize & INCREMENTAL) {
      update.clear();
      std::unordered_set<int> update_temp;
      for (const auto &input : inputs) {
        for (const auto &up : input->update) {
          update_temp.insert(up);
        }
      }
      for (const auto &up : update_temp) {
        int now = 0;
        for (int j = input_num - 1; j >= 0; j--) {
          now = (now << 1) + inputs[j]->data[up];
        }
        int temp = lookup_table[now];

        if (temp != data[up]) {
          data[up] = temp;
          update.insert(up);
        } else {
        }
      }
    }

#if LOG
    std::cout << "LookupTableNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }
};

/**
 * @brief Now only support 2 inputs and 1 output in binary form
 *
 */
class SubcircuitNode : public Node {
public:
  std::vector<Node *> starts;
  std::vector<Node *> ends;
  int LEN;

private:
  std::vector<Node *> components;
  std::unordered_map<int, int> lookup_table;

  // class used to generate lookup table
  class DataNode : public Node {
  protected:
    DataNode *next = nullptr;

  public:
    DataNode(int _input_num, int _output_num, const Config &_conf)
        : Node(_input_num, _output_num, _conf) {}

    void set_next_node(DataNode *_next) { next = _next; }

    virtual Node *copy() {
      // assert(0 && "Not implemented");
      return this;
    }

    std::string label() { return "SubcircuitNode::DataNode"; }
    std::string codegen_str() {
      assert(0 && "Not implemented");
      return "";
    }

    void set_data(std::vector<int> _data) { data = _data; }

    void init(const std::vector<std::string> &extra){};

    bool verilog_has_instance() override { return false; }

    std::string
    verilog_codegen_str(std::unordered_map<Node *, int> node_index_map,
                        std::unordered_map<int, std::vector<std::vector<int>>>
                            lfsr_shared_map) {
      return "";
    }

    void reset_zero() {
      // assert(0 && "Not implemented");
      clear_ready();
      // data = std::vector<int>(1, 0);

#if LOG
      std::cout << "-----"
                << "SourceNode reset_zero(): " << data[0] << "-----"
                << std::endl;
#endif

      ready = true;
      notify_reset_zero();
      // it should not call next->reset_zero(), because the next source node
      // will do it itself in its run() function.
    };

    void run() {
      clear_ready();
#if LOG
      std::cout << "-----"
                << "SubcircuitNode::DataNode:";
      print_vector(std::cout, data);
      std::cout << "-----" << std::endl;
#endif
      ready = true;
      notify();
      // Recursively assign all DataNodes
      if (next != nullptr) {
        next->run();
      }
      // clear_ready();
    }
  };

public:
  SubcircuitNode(int _input_num, int _output_num, const Config &_conf)
      : Node(_input_num, _output_num, _conf) {
    LEN = (int) std::pow(2, conf.bit_width);
  }

  ~SubcircuitNode() {
    for (auto &input : input_nodes) {
      delete input;
    }
  }

  std::vector<std::string> start_nodes_name;
  std::vector<std::string> end_nodes_name;
  std::vector<DataNode *> input_nodes;

  bool verilog_has_instance() override { return false; }

  virtual Node *copy() {
    // std::cout << "Copy in subcircuit node." << std::endl;
    SubcircuitNode *node =
        new SubcircuitNode(this->input_num, this->output_num, this->conf);

    // Needn't copy DataNodes
    node->input_nodes = this->input_nodes;
    // The change of starts and ends vectors are down in the copy() of graph
    // class
    node->starts = this->starts;
    node->ends = this->ends;

    return node;
  }

  std::string label() { return "SUBCIRCUIT"; }
  std::string codegen_str() {
    std::string ans = "    SubcircuitNode *<-name-> = new SubcircuitNode(" +
                      std::to_string(input_num) + ", " +
                      std::to_string(output_num) + ", conf" + ");";
    ans += "\n    std::vector<Node *> <-name->_start_nodes;";
    for (const auto &name : start_nodes_name) {
      ans += "\n    <-name->_start_nodes.push_back(" + name + ");";
    }

    ans += "\n    std::vector<Node *> <-name->_end_nodes;";
    for (const auto &name : end_nodes_name) {
      ans += "\n    <-name->_end_nodes.push_back(" + name + ");";
    }
    // ans += "\n    <-name->->add_inside_circuits(<-name->_start_nodes, "
    //        "<-name->_end_nodes);";
    return ans;
  }
  std::string codegen_str_after() {
    return "\n    <-name->->add_inside_circuits(<-name->_start_nodes, "
           "<-name->_end_nodes);";
  }

  void add_components(std::vector<Node *> _components) {
    components = _components;
  }

  void add_inside_circuits(std::vector<Node *> _starts,
                           std::vector<Node *> _ends) {
    starts = _starts;
    ends = _ends;

    int index = 0;
    for (const auto &start : starts) {
      for (int i = 0; i < start->input_num; i++) {
        input_nodes.push_back(new DataNode(0, 1, conf));
        input_nodes[index]->connect(0, start, i);
        index++;
      }
    }
    for (int i = 0; i < input_num; i++) {
      if (i < input_num - 1)
        input_nodes[i]->set_next_node(input_nodes[i + 1]);
    }
  }

  void reset() {
    if (conf.optimize & SUBCIRCUIT) {
      input_nodes[0]->data = std::vector<int>(1);
      input_nodes[1]->data = std::vector<int>(1);
      lookup_table.clear();
      //       int LEN = int(pow(2, conf.bit_width));
      //       for (int i = 0; i < LEN; i++) {
      //         input_nodes[0]->data = std::vector(1, i);
      //         for (int j = 0; j < LEN; j++) {
      //           input_nodes[1]->data = std::vector(1, j);

      //           input_nodes[0]->run();
      //           std::vector<int> data_temp = ends[0]->data;

      // #if LOG
      //           std::cout << "-----"
      //                     << "Subcircuit result:";
      //           print_vector(std::cout, data_temp);
      //           std::cout << "-----" << std::endl;
      // #endif

      //           lookup_table[i * 100000 + j] = data_temp[0];
      //           // std::cout << "data_temp: " << data_temp[0] << std::endl;
      //         }
      //       }
    }
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(inputs[0]->data);

    if (conf.optimize & SUBCIRCUIT) {
      for (int i = 0; i < data.size(); i++) {
        int number =
            (inputs[0]->data[i] + LEN) * 100000 + (inputs[1]->data[i] + LEN);
        if (!lookup_table.contains(number)) {
          input_nodes[0]->data[0] = inputs[0]->data[i];
          input_nodes[1]->data[0] = inputs[1]->data[i];
          input_nodes[0]->run();
          lookup_table[number] = ends[0]->data[0];
        }
        data[i] = lookup_table[number];
      }
    } else {
      for (int i = 0; i < inputs.size(); i++) {
        input_nodes[i]->data = inputs[i]->data;
      }
      for (const auto node : input_nodes) {
        node->reset_zero();
      }
    }
#if LOG
    std::cout << "SubcircuitNode reset_zero(): ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif
    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(1, 0);

    if (conf.optimize & SUBCIRCUIT) {
      for (int i = 0; i < data.size(); i++) {
        int number =
            (inputs[0]->data[i] + LEN) * 100000 + (inputs[1]->data[i] + LEN);
        if (!lookup_table.contains(number)) {
          input_nodes[0]->data[0] = inputs[0]->data[i];
          input_nodes[1]->data[0] = inputs[1]->data[i];
          input_nodes[0]->run();
          lookup_table[number] = ends[0]->data[0];
        }
        data[i] = lookup_table[number];
      }
    } else {
      for (int i = 0; i < inputs.size(); i++)
        input_nodes[i]->data = inputs[i]->data;
      input_nodes[0]->run();

      data[0] = ends[0]->data[0];
    }

#if LOG
    std::cout << "SubcircuitNode: ";
    print_vector(std::cout, data);
    std::cout << std::endl;
#endif

    ready = true;
    notify();
  }
};

#endif