#include "Modules/Var.h"

void interpolation(const Config &conf) {
  SC_sim::Var t11(SC_sim::SC, conf);
  SC_sim::Var t12(SC_sim::SC, conf);
  SC_sim::Var t21(SC_sim::SC, conf);
  SC_sim::Var t22(SC_sim::SC, conf);

  SC_sim::Var x(SC_sim::SC, conf);
  SC_sim::Var y(SC_sim::SC, conf);

  SC_sim::Var res1 = SC_sim::Op::ScalingAdd(t11, t12, x);
  SC_sim::Var res2 = SC_sim::Op::ScalingAdd(t21, t22, x);

  SC_sim::Var res = SC_sim::Op::ScalingAdd(res2, res1, y);

  res.reorderInput({t11, t12, x, t21, t22, y});
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return (1 - ins[2]) * ins[5] * ins[0] + ins[2] * ins[5] * ins[1] +
           (1 - ins[2]) * (1 - ins[5]) * ins[3] +
           ins[2] * (1 - ins[5]) * ins[4];
  });
  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1000;
  conf.graph_name = "interpolation";
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

  interpolation(conf);
}