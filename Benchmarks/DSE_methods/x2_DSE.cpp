#include "Modules/Var.h"
#include <cmath>
#include <ginac/symbol.h>

void x2(Config conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::SNG_TYPE::LFSR_SNG, NULL,
                GiNaC::symbol("x"));

  SC_sim::Var res = x * SC_sim::Op::DFF(x, 3);

  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return std::pow(ins[0], 2);
  });

  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1000;
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

  x2(conf);
}