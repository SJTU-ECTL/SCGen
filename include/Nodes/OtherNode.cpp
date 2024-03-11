#include "OtherNode.h"
#include "Nodes/BinaryNode.h"
#include "Nodes/SourceNode.h"

DestNode::DestNode(int _input_num, int _output_num, const Config &_conf)
    : Node(_input_num, _output_num, _conf) {
  assert(_input_num == 1);
  in_sc_core = false;
}

std::string
DestNode::verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
  std::string wire_str = verilog_wire_name(inputs[0], this);
  // unipolar
  std::string out;
  if (conf.scType == unipolar) {
    out = "\t\t\tb_dest <= " + wire_str + " + b_dest;\n";
  } else {
    out = "\t\t\tif (" + wire_str +
          ") begin\n"
          "\t\t\t\tb_dest <= b_dest + 1;\n"
          "\t\t\tend\n"
          "\t\t\telse begin\n"
          "\t\t\t\tb_dest <= b_dest - 1;\n"
          "\t\t\tend\n";
  }

  return "\t//Circuit Output logic\n"
         "\talways @(posedge clk) begin\n"
         "\t\tif (rst) begin\n"
         "\t\t\tb_dest <= 0;\n"
         "\t\tend\n"
         "\t\telse begin\n" +
         out +
         "\t\tend\n"
         "\tend\n";
}

std::string DestNode::verilog_codegen_multi_output(
    std::unordered_map<Node *, int> node_index_map) {
  return "";
}

std::string DestNode::codegen_str() {
  return "    DestNode *<-name-> = new DestNode(" + std::to_string(input_num) +
         ", " + std::to_string(output_num) + ", conf" + ");" +
         "\n    outputs.push_back(<-name->);";
}

Node *DestNode::copy() {
  Node *node = new DestNode(this->input_num, this->output_num, this->conf);
  node->id = this->id;
  return node;
}

void DestNode::reset_zero() {
  if (ready || !is_ready())
    return;

  update.clear();

  data = std::vector<int>(1);
  if (conf.scType == unipolar)
    data[0] = count_arr(inputs[0]->data, 0, inputs[0]->data.size());
  else
    data[0] = count_arr_1_0(inputs[0]->data, 0, inputs[0]->data.size());

  previous = inputs[0]->data;

#if LOG
  std::cout << "DestNode reset_zero(): " << data[0] << std::endl;
#endif

  ready = true;
  notify_reset_zero();
}

void DestNode::run() {
  if (ready || !is_ready())
    return;
  if (conf.optimize & INCREMENTAL) {
    int pre = data[0];
    update.clear();
    for (const auto &up : inputs[0]->update) {
      if (inputs[0]->data[up] != previous[up]) {
        data[0] += inputs[0]->data[up] - previous[up];
        previous[up] = inputs[0]->data[up];
      }
    }
    // Indicating the current data stored in DestNode (the sum) is changed
    if (pre != data[0])
      update.insert(0);
  } else {
    data.resize(1);
    if (conf.scType == unipolar)
      data[0] = count_arr(inputs[0]->data, 0, inputs[0]->data.size());
    else
      data[0] = count_arr_1_0(inputs[0]->data, 0, inputs[0]->data.size());
  }
  if (conf.optimize & LOOKUPTABLE) {
    if (!(conf.optimize & INCREMENTAL)) {
      data.resize(1);
      if (conf.scType == unipolar)
        data[0] = count_arr(inputs[0]->data, 0, inputs[0]->data.size());
      else
        data[0] = count_arr_1_0(inputs[0]->data, 0, inputs[0]->data.size());
    }
  }
#if LOG
  std::cout << "DestNode: " << data[0] << std::endl;
#endif

  ready = true;
  notify();
}

BinaryDestNode::BinaryDestNode(int _input_num, int _output_num,
                               const Config &_conf)
    : DestNode(_input_num, _output_num, _conf) {
  assert(_input_num == 1);
  in_sc_core = false;
}

Node *BinaryNode::copy() {
  Node *node =
      new BinaryDestNode(this->input_num, this->output_num, this->conf);
  node->id = this->id;
  return node;
}

Node *BinaryDestNode::copy() {
  Node *node =
      new BinaryDestNode(this->input_num, this->output_num, this->conf);
  node->id = this->id;
  return node;
}

std::string BinaryDestNode::codegen_str() {
  return "    BinaryDestNode *<-name-> = new BinaryDestNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf" + ");" + "\n    outputs.push_back(<-name->);";
}

std::string BinaryDestNode::verilog_codegen_str(
    std::unordered_map<Node *, int> node_index_map) {
  std::string wire_str = verilog_wire_name(inputs[0], this);
  return "\t//Circuit Output logic (BinaryDestNode)\n"
         "\tassign b_dest = " +
         wire_str + ";\n";
}

std::string BinaryDestNode::verilog_codegen_multi_output(
    std::unordered_map<Node *, int> node_index_map) {
  return "";
}

void BinaryDestNode::run() {
  if (ready || !is_ready())
    return;

  data = std::vector<int>(1);
  data[0] = inputs[0]->data[0];

  ready = true;
  notify();
}
