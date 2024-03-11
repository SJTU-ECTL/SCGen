#include "Modules/Var.h"
#include <ginac/symbol.h>

using namespace SC_sim;

void roberts(const Config &conf) {
  auto type = SC_sim::LFSR_SNG;

  SC_sim::Var x11(SC_sim::SC, conf, type, NULL, GiNaC::symbol("x11"));
  SC_sim::Var x12(SC_sim::SC, conf, type, NULL, GiNaC::symbol("x12"));
  SC_sim::Var x22(SC_sim::SC, conf, type, &x11, GiNaC::symbol("x22"));
  SC_sim::Var x21(SC_sim::SC, conf, type, &x12, GiNaC::symbol("x21"));

  SC_sim::Var res1 = x11.abs_sub(x22);   // |x11-x22|
  SC_sim::Var res2 = x21.abs_sub(x12);

  SC_sim::Var res = res1 + res2;

  res.reorderInput({x11, x22, x21, x12});
  res.addCalculate("min", [](const std::vector<double> &ins) {
    return (abs(ins[0] - ins[1]) + abs(ins[2] - ins[3])) / 2;
  });

  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1000;
  conf.graph_name = "roberts";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.rand_till_converge = false;
  conf.seed = rand();

  conf.bit_width = 6;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  roberts(conf);
}
