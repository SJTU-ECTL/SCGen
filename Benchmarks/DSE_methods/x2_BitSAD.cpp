#include "Modules/Var.h"
#include <ginac/symbol.h>

/**
  BitSAD v2 only generates fixed design of LFSR SNG at 20 and 32 bit widths.
  Here we generate randomly configured LFSRs instead.
*/

void x2(Config conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::SNG_TYPE::LFSR_SNG, NULL,
                GiNaC::symbol("x"));

  SC_sim::Var res = x * SC_sim::Op::DFF(x, 3);

  res.addCalculate(
      "MAE", [](const std::vector<double> &ins) { return ins[0] * ins[0]; });

  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1;
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