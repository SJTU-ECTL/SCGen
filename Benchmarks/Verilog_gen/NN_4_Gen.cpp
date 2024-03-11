#include "Modules/Var.h"

using namespace SC_sim;

int main() {
  Config conf;
  conf.rand_times = 10;
  conf.graph_name = "NN_4";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.seed = rand();
  conf.scType = bipolar;
  conf.bit_width = 8;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  std::vector<Var> x(4);
  std::vector<Var> y(4);
  std::vector<double> para = {0.0625, 0.125, 0.0625, 0.125};
  std::vector<Var> tem(4);

  for (int i = 0; i < 4; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf, LFSR_SNG);
    tem[i] = x[i] * y[i];
  }

  Var res = Op::TANH(Var::Randomizer(Var::sum(tem)), 2);

  x.insert(x.end(), y.begin(), y.end());   // x is now used as inputs sequence
  res.reorderInput(x);
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return tanh(2 * (ins[0] * ins[4] + ins[1] * ins[5] + ins[2] * ins[6] +
                     ins[3] * ins[7]));
  });

  auto result = res.simulate();
  res.config_by_str(result.second);
  res.genVerilog();
}