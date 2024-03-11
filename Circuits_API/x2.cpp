#include "Modules/Var.h"
#include <ginac/symbol.h>

void x2(Config conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::SNG_TYPE::LFSR_SNG, NULL,
                GiNaC::symbol("x"));

  SC_sim::Var res = x * SC_sim::Op::DFF(x, 3);

  res.reorderInput({x});

  res.addCalculate(
      "MAE", [](const std::vector<double> &ins) { return ins[0] * ins[0]; });

  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.inserting_zero = 0;
  conf.leq = 0;
  conf.bit_width = 6;
  conf.rand_times = 1000000;
  conf.simType = full_sim;
  conf.rand_till_converge = false;
  conf.graph_name = "x2_DFF";
  conf.optimize = 0;
  conf.seed = rand();
  if (argc > 1)
    conf.search_method = argv[1];
  else
    conf.search_method = "RANDOM";

  int bit_widths[4] = {6, 7, 8, 9};
  for (auto bit_width : bit_widths) {
    conf.bit_width = bit_width;

    conf.optimize = 0;
    x2(conf);

    conf.optimize = 1;
    x2(conf);

    conf.optimize = 4;
    x2(conf);

    conf.optimize = 5;
    x2(conf);
  }
}