#include "Modules/Var.h"
#include <ginac/symbol.h>

void x_plus_y(const auto &conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::LFSR_SNG, NULL, GiNaC::symbol("x"));
  SC_sim::Var y(SC_sim::SC, conf, SC_sim::LFSR_SNG, NULL, GiNaC::symbol("y"));

  SC_sim::Var res = x + y;

  res.reorderInput({x, y});
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return (ins[0] + ins[1]) / 2;
  });

  res.simulate();
}

int main() {
  Config conf;
  conf.inserting_zero = 0;
  conf.leq = 0;
  conf.bit_width = 6;
  conf.rand_times = 10000;
  conf.simType = full_sim;
  conf.rand_till_converge = false;
  conf.graph_name = "x_plus_y";
  conf.optimize = 0;
  conf.seed = rand();
  conf.search_method = "RANDOM";

  int bit_widths[4] = {6, 7, 8, 9};
  for (auto bit_width : bit_widths) {
    conf.bit_width = bit_width;

    conf.optimize = 0;
    x_plus_y(conf);

    conf.optimize = 1;
    x_plus_y(conf);

    conf.optimize = 4;
    x_plus_y(conf);

    conf.optimize = 5;
    x_plus_y(conf);
  }
}