#ifndef DISPLAYNODE_H
#define DISPLAYNODE_H

#include "Node.h"
#include "Utils/Utils.h"

class DisplayNode : public Node {
public:
  DisplayNode(int _input_num, int _output_num, const Config &_conf)
      : Node(_input_num, _output_num, _conf) {
    assert(_input_num == 1);
  }

  virtual Node *copy() {
    assert(0 && "Not implemented");
    return nullptr;
  }

  std::string label() { return "Display"; }
  std::string codegen_str() {
    return "    DisplayNode *<-name-> = new DisplayNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");";
  }

  bool verilog_has_instance() override { return false; }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) override {
    return "";
  }
  std::string verilog_codegen_multi_output(
      std::unordered_map<Node *, int> node_index_map) override {
    return "";
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;

#if LOG
    std::cout << "DisplayNode reset_zero(): " << std::endl;
#endif

    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;

    std::cout << inputs[0]->data[0] << std::endl;
#if LOG
    std::cout << "DisplayNode: " << data[0] << std::endl;
#endif

    ready = true;
    notify();
  }
};

#endif