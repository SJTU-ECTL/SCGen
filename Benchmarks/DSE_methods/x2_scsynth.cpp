#include "Modules/Var.h"
#include <ginac/symbol.h>

/**
  For single LFSR, the generation code of LFSR is in
  `VerilogSCWrapperGenerator.m`.

  The seed is 1, and the polynomial is determined in `VerilogLFSRGenerator.m`.

  Conversion is needed for polynomial configuration:
    Removing the first number in scsynth polynomial
    For next numbers: SCGen_num = scsynth_num - 1
*/

std::string get_scsynth_config(int bitwidth) {
  // clang-format off
  switch (bitwidth) {
  case 6:
    return "config of scsynth of bit 6\n"
           "1: node id: 1, polynomial: [4], seed: [0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0";
  case 7:
    return "config of scsynth of bit 7\n"
           "1: node id: 1, polynomial: [5], seed: [0, 0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0";
  case 8:
    return "config of scsynth of bit 8\n"
           "1: node id: 1, polynomial: [0, 5, 6], seed: [0, 0, 0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0";
  case 9:
    return "config of scsynth of bit 9\n"
           "1: node id: 1, polynomial: [4], seed: [0, 0, 0, 0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0";
  }
  return "";
  // clang-format on
}

void x2(Config conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::SNG_TYPE::LFSR_SNG, NULL,
                GiNaC::symbol("x"));

  SC_sim::Var res = x * SC_sim::Op::DFF(x, 3);

  res.addCalculate(
      "MAE", [](const std::vector<double> &ins) { return ins[0] * ins[0]; });

  std::string config = get_scsynth_config(conf.bit_width);
  auto result = res.simulate_conf(config);
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 10;
  conf.graph_name = "x2_DFF";
  conf.simType = full_sim;
  conf.rand_till_converge = false;
  conf.seed = rand();

  conf.bit_width = 6;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  std::vector<int> bit_widths = {6, 7, 8, 9};
  for (int bit_width : bit_widths) {
    conf.bit_width = bit_width;
    x2(conf);
  }
}