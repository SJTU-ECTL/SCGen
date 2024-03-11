
#ifndef BINARYNODE_H
#define BINARYNODE_H

#include "Node.h"
#include "OtherNode.h"
#include "SNGNode.h"
#include "Utils/Utils.h"
#include <cassert>
#include <fstream>

class BinaryNode : public Node {
public:
  BinaryNode(int _input_num, int _output_num, const Config &_conf)
      : Node(_input_num, _output_num, _conf){};

  virtual Node *copy() = 0;
};
class BinaryNormAddNode : public BinaryNode {
protected:
  int N;
  int LEN;

public:
  BinaryNormAddNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {
    N = conf.bit_width;
    LEN = (int) std::pow(2, N);
  }

  virtual Node *copy() {
    Node *node =
        new BinaryNormAddNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string label() { return "BinaryNormAdd"; }
  std::string codegen_str() {
    return "    BinaryNormAddNode *<-name-> = new BinaryNormAddNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf);";
  }

  void run() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    data[0] = (inputs[0]->data[0] + inputs[1]->data[0]);

    ready = true;
    notify();
  }
};

class BinaryAddNode : public BinaryNode {
protected:
  int N;
  int LEN;

public:
  BinaryAddNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {
    N = conf.bit_width;
    LEN = (int) std::pow(2, N);
  }

  virtual Node *copy() {
    Node *node =
        new BinaryAddNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string label() { return "BinaryAdd"; }
  std::string codegen_str() {
    return "    BinaryAddNode *<-name-> = new BinaryAddNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf);";
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;

    data = std::vector<int>(1, 0);
    data[0] = inputs[0]->data[0]*(1-(double)inputs[2]->data[0]/LEN) + inputs[1]->data[0]*(double)inputs[2]->data[0]/LEN;
#if LOG
    std::cout << "BinaryAdd reset_zero(): ";
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
    data[0] = inputs[0]->data[0]*(1-(double)inputs[2]->data[0]/LEN) + inputs[1]->data[0]*(double)inputs[2]->data[0]/LEN;

    ready = true;
    notify();
  }
};

class BinarySumNode : public BinaryNode {
protected:
  int N;
  int LEN;

public:
  BinarySumNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {
    N = conf.bit_width;
    LEN = (int) std::pow(2, N);
  }

  virtual Node *copy() {
    Node *node =
        new BinarySumNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }
  std::string label() { return "BinarySum"; }
  std::string codegen_str() {
    return "    BinarySumNode *<-name-> = new BinarySumNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf);";
  }
  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    std::ofstream apc_file("../Verilog/" + conf.graph_name + "/BinarySum" +
                           std::to_string(node_index_map[this]) + ".v");

    apc_file << "module BinarySum" << std::to_string(node_index_map[this])
             << " (\n";
    for (int i = 0; i < this->input_num; i++)
      apc_file << "\ti" << std::to_string(i + 1) << ",\n";
    apc_file << "\to1\n"
                ");\n"
                "\tparameter bit_width = 6;\n";
    for (int i = 0; i < this->input_num; i++)
      apc_file << "\tinput [bit_width -1 :0] i" << std::to_string(i + 1)
               << ";\n";
    apc_file << "\toutput [bit_width -1 :0] o1;\n";

    apc_file << "\tassign o1 = ";
    for (int i = 0; i < this->input_num - 1; i++)
      apc_file << "i" << std::to_string(i + 1) << " +";
    apc_file << "i" << std::to_string(input_num)
             << ";\n"
                "endmodule";

    return "\t" + label() + std::to_string(node_index_map[this]) +
           " #(.bit_width(bit_width)) " + label() + "_" +
           std::to_string(node_index_map[this]) + "(\n" +
           verilog_codegen_io_str(node_index_map) + "\t);\n";
  }

  void run() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(1);
    for (auto input : inputs)
      data[0] += input->data[0];
    ready = true;
    notify();
  }
};

class BinaryAbsSubNode : public BinaryNode {
protected:
  int N;
  int LEN;

public:
  BinaryAbsSubNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {
    N = conf.bit_width;
    LEN = (int) std::pow(2, N);
  }
  virtual Node *copy() {
    Node *node =
        new BinaryAbsSubNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string label() { return "BinaryAbsSub"; }
  std::string codegen_str() {
    return "    BinaryAbsSubNode *<-name-> = new BinaryAbsSubNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf);";
  }

  void run() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(1, 0);
    data[0] = abs(inputs[0]->data[0] - inputs[1]->data[0]);
    ready = true;
    notify();
  }
};

class BinaryMulNode : public BinaryNode {
protected:
  int N;
  int LEN;

public:
  BinaryMulNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {
    N = conf.bit_width;
    LEN = (int) std::pow(2, N);
  }

  virtual Node *copy() {
    Node *node =
        new BinaryMulNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string label() { return "BinaryMul"; }
  std::string codegen_str() {
    return "    BinaryMulNode *<-name-> = new BinaryMulNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf);";
  }

  void run() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    data[0] = inputs[0]->data[0] * inputs[1]->data[0] / LEN;
    // data[0] = inputs[0]->data[0] * inputs[1]->data[0];

    ready = true;
    notify();
  }
};

class APCNode : public BinaryNode {
public:
  APCNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {}

  virtual Node *copy() {
    Node *node = new APCNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string label() { return "APC"; }
  std::string codegen_str() {
    return "    APCNode *<-name-> = new APCNode(" + std::to_string(input_num) +
           ", " + std::to_string(output_num) + ", conf" + ");";
  }

  std::string verilog_module_init_str() override;

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) override;

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(1, 0);   // output is binary
    for (const auto &input : inputs) {
      for (int bit : input->data) {
        if (conf.scType == unipolar)
          data[0] += bit;   // count all 1s
        else                // bipolar
          data[0] += bit == 1 ? 1 : -1;
      }
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
    data = std::vector<int>(1, 0);   // output is binary
    for (int i = 0; i < inputs[0]->data.size(); i++) {
      for (auto input : inputs) {
        if (conf.scType == unipolar)
          data[0] += input->data[i];   // count all 1s
        else                           // bipolar
          data[0] += (input->data[i] == 1) ? 1 : -1;
      }
    }

    ready = true;
    notify();
  }
};

class CounterNode : public BinaryNode {
public:
  CounterNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {
    assert(_input_num == 1);
  }
  std::string label() { return "Counter"; };
  std::string codegen_str() {
    return "    CounterNode *<-name-> = new CounterNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");";
  }

  virtual Node *copy() {
    Node *node = new CounterNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "\t" + label() + " #(.bit_width(bit_width)) " + label() + "_" +
           std::to_string(node_index_map[this]) + "(\n" +
           verilog_codegen_io_str(node_index_map) + ",\n\t\treset," + "clk" +
           ");\n";
  }

  void run() {
    data = std::vector<int>(1);
    if (conf.scType == unipolar) {
      data[0] = count_arr(inputs[0]->data, 0, inputs[0]->data.size());
    } else {
      data[0] = count_arr_1_0(inputs[0]->data, 0, inputs[0]->data.size());
    }

    // TODO: optimization

    ready = true;
    notify();
  }
};

class BinaryTanhNode : public BinaryNode {
protected:
  int k;
  int N;
  int LEN;

public:
  BinaryTanhNode(int _input_num, int _output_num, const Config &_conf,
                 int _k = 1)
      : BinaryNode(_input_num, _output_num, _conf) {
    k = _k;
    N = conf.bit_width;
    LEN = (int) std::pow(2, N);
  }
  virtual Node *copy() {
    BinaryTanhNode *node =
        new BinaryTanhNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    this->k = node->k;
    return node;
  }

  std::string label() { return "BinaryTanh"; }
  std::string codegen_str() {
    return "    BinaryTanhNode *<-name-> = new BinaryTanhNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf, " + std::to_string(k) + ");";
  }

  void run() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    data[0] = tanh(inputs[0]->data[0] / LEN) * LEN;

    ready = true;
    notify();
  }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "\t" + label() + " #(.bit_width(bit_width),.scaling_factor(" +
           std::to_string(this->k) + ")) " + label() + "_" +
           std::to_string(node_index_map[this]) + "(\n" +
           verilog_codegen_io_str(node_index_map) + ");\n";
  }
};

class BinarySaturAddNode : public BinaryNode {
protected:
  int N;
  int LEN;

public:
  BinarySaturAddNode(int _input_num, int _output_num, const Config &_conf)
      : BinaryNode(_input_num, _output_num, _conf) {
    N = conf.bit_width;
    LEN = (int) std::pow(2, N);
  }

  virtual Node *copy() {
    Node *node =
        new BinarySaturAddNode(this->input_num, this->output_num, this->conf);
    node->id = this->id;
    return node;
  }

  std::string label() { return "BinarySaturAddNode"; }
  std::string codegen_str() {
    return "    BinarySaturAddNode *<-name-> = new BinarySaturAddNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");";
  }

  std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    std::string input_str = "\t\t{";
    size_t input_index = 0;
    for (; input_index < inputs.size() - 1; input_index++) {
      if (node_index_map.contains(
              inputs[input_index]))   // ensure Node such as CalculateNode is
                                      // not considered
        input_str += "wire_" +
                     std::to_string(node_index_map[inputs[input_index]]) + "_" +
                     std::to_string(node_index_map[this]) + ",";
    }
    if (node_index_map.contains(
            inputs[input_index]))   // ensure Node such as CalculateNode is not
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

  void reset_zero() {
    if (ready || !is_ready())
      return;
    data = std::vector<int>(inputs[0]->data);
    for (const auto &i : inputs) {
      for (size_t index = 0; index < i->data.size(); ++index) {
        data[index] = data[index] + i->data[index];
      }
    }
    // saturation
    for (int &index : data) {
      if (index >= LEN) {
        index = LEN - 1;
      }
    }
#if LOG
    std::cout << "BinarySaturAddNode reset_zero(): ";
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
      for (const auto &i : inputs) {
        for (size_t index = 0; index < i->data.size(); ++index) {
          data[index] = data[index] + i->data[index];
        }
      }
      // saturation
      for (int &index : data) {
        if (index >= LEN) {
          index = LEN - 1;
        }
      }
    } else if (conf.optimize == INCREMENTAL) {
      assert(1);
    }
#if LOG
    std::cout << "BinarySaturAddNode: ";
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
        data[index] = data[index] + i->data[index];
      }
    }
    // saturation
    for (int &index : data) {
      if (index >= LEN) {
        index = LEN - 1;
      }
    }

    ready = true;
    notify_simple();
  }
};

#endif
