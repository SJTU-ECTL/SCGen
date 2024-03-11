#include "Modules/Var.h"
#include <ginac/symbol.h>

int main() {
  Config conf;
  conf.rand_times = 10;
  conf.graph_name = "interpolation";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.seed = rand();
  conf.bit_width = 8;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  SC_sim::Var t11(SC_sim::SC, conf, SC_sim::LFSR_SNG, nullptr, GiNaC::symbol("t_11"));
  SC_sim::Var t12(SC_sim::SC, conf, SC_sim::LFSR_SNG, nullptr, GiNaC::symbol("t_12"));
  SC_sim::Var t21(SC_sim::SC, conf, SC_sim::LFSR_SNG, nullptr, GiNaC::symbol("t_21"));
  SC_sim::Var t22(SC_sim::SC, conf, SC_sim::LFSR_SNG, nullptr, GiNaC::symbol("t_22"));

  SC_sim::Var x(SC_sim::SC, conf, SC_sim::LFSR_SNG, nullptr, GiNaC::symbol("x"));
  SC_sim::Var y(SC_sim::SC, conf, SC_sim::LFSR_SNG, nullptr, GiNaC::symbol("y"));

  SC_sim::Var res1 = SC_sim::Op::ScalingAdd(t11, t12, x);   // t11 * (1 - x) + t12 * x
  SC_sim::Var res2 = SC_sim::Op::ScalingAdd(t21, t22, x);   // t21 * (1 - x) + t22 * x
  SC_sim::Var res = SC_sim::Op::ScalingAdd(res2, res1, y);

  res.reorderInput({t11, t12, x, t21, t22, y});
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return (ins[0] * (1 - ins[2]) + ins[1] * ins[2]) * ins[5] +
           (ins[3] * (1 - ins[2]) + ins[4] * ins[2]) * (1 - ins[5]);
  });

  auto result = res.simulate();
  res.config_by_str(result.second);
  res.genVerilog();
}