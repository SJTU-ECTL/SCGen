#include "Modules/Var.h"
#include <ginac/symbol.h>

int main() {
  Config conf;
  conf.rand_times = 1000;
  conf.graph_name = "x_plus_y";
  conf.simType = full_sim;
  conf.seed = rand();
  conf.bit_width = 8;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  SC_sim::Var x(SC_sim::SC, conf, SC_sim::LFSR_SNG, NULL, GiNaC::symbol("x"));
  SC_sim::Var y(SC_sim::SC, conf, SC_sim::LFSR_SNG, NULL, GiNaC::symbol("y"));
  SC_sim::Var res = x + y;

  res.reorderInput({x, y});
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return (ins[0] + ins[1]) / 2;
  });

  auto result = res.simulate();
  res.config_by_str(result.second);
  res.genVerilog();
}