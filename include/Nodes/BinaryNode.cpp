#include "Nodes/BinaryNode.h"
#include <sstream>

std::string APCNode::verilog_module_init_str() {
  std::stringstream str;
  str << "\t" << label() << this->id << " #(.bit_width(bit_width)) " << label()
      << "_" << this->id << "(\n";
  str << "\t\tclk,\n"
      << "\t\trst,\n";
  str << verilog_codegen_io_str({}) << "\n";
  str << "\t);\n";
  return str.str();
}

std::string
APCNode::verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
  std::ofstream apc_file("../Verilog/" + conf.graph_name + "/APC" +
                         std::to_string(this->id) + ".v");

  apc_file << "module APC" << this->id << " (\n"
           << "\tclk,\n"
           << "\trst,\n";

  for (int i = 0; i < this->input_num; i++)
    apc_file << "\ti" << (i + 1) << ",\n";
  apc_file << "\to1\n"
              ");\n"
              "\tparameter bit_width = "
           << conf.bit_width << ";\n"
           << "\n";

  apc_file << "\tinput clk;\n"
           << "\tinput rst;\n";
  for (int i = 0; i < this->input_num; i++)
    apc_file << "\tinput i" << (i + 1) << ";\n";
  apc_file << "\toutput reg [bit_width-1:0] o1;\n"
           << "\n";

  if (conf.scType == unipolar) {
    apc_file << "\talways @(posedge clk or posedge rst) begin\n"
                "\t\tif (rst) o1 <= 0;\n"
                "\t\telse o1 <= o1 +";
    for (int i = 0; i < this->input_num - 1; i++)
      apc_file << " i" << (i + 1) << " +";

    apc_file << " i" << input_num
             << ";\n"
                "\tend\n"
                "\n";

    apc_file << "endmodule";
  } else {
    apc_file << "\talways @(posedge clk or posedge rst) begin\n"
                "\t\tif (rst) o1 <= 0;\n"
                "\t\telse begin\n";

    for (int i = 0; i < this->input_num; i++)
      apc_file << "\t\t\tif (i" << (i + 1)
               << " == 1) o1 <= o1 + 1; else o1 <= o1 - 1;\n";

    apc_file << "\t\tend\n"
                "\tend\n"
                "\n";

    apc_file << "endmodule";
  }
  return "";
}