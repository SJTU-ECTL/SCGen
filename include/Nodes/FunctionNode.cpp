#include "Nodes/FunctionNode.h"
#include <sstream>

std::string SCTanhNode::verilog_module_init_str() {
  std::stringstream str;
  str << "\t" << label() << " #(.bit_width(bit_width),.state_bit_width("
      << int(sqrt(N)) << "),.state_num(" << N << ")) " << label() << "_"
      << this->id << "(\n";

  str << "\t\tclk,\n"
      << "\t\trst,\n"
      << verilog_codegen_io_str({}) << "\n"
      << "\t);\n";
  return str.str();
}

std::string XnorNode::verilog_module_init_str() {
  std::stringstream str;
  str << "\t" << label() << " #(.bit_width(bit_width),.input_num("
      << this->inputs.size() << ")) " << label() << "_" << this->id << "(\n";

  // Input wires
  str << "\t\t{";
  for (int i = 0; i < inputs.size() - 1; i++) {
    str << verilog_wire_name(inputs[i], this) + ", ";
  }
  str << verilog_wire_name(inputs[inputs.size() - 1], this);
  str << "},\n";

  // Output
  str << "\t\t" << verilog_wire_name(this, outputs[0]) + "\n";
  str << "\t);\n";
  return str.str();
}