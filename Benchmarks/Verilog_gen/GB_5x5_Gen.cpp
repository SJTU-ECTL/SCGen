#include "Modules/Var.h"
#include <string>

using namespace SC_sim;

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 10;
  conf.graph_name = "GB_5x5";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.seed = rand();
  conf.bit_width = 8;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  int dim = 5;
  std::vector<Var> x(dim * dim);
  std::vector<Var> y(dim * dim);
  std::vector<double> para = {
      1.0 / 273, 4.0 / 273,  7.0 / 273,  4.0 / 273,  1.0 / 273,
      4.0 / 273, 16.0 / 273, 26.0 / 273, 16.0 / 273, 4.0 / 273,
      7.0 / 273, 26.0 / 273, 41.0 / 273, 26.0 / 273, 7.0 / 273,
      4.0 / 273, 16.0 / 273, 26.0 / 273, 16.0 / 273, 4.0 / 273,
      1.0 / 273, 4.0 / 273,  7.0 / 273,  4.0 / 273,  1.0 / 273};
  std::vector<Var> tem(dim * dim);

  for (int i = 0; i < dim * dim; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf, LFSR_SNG);
    tem[i] = x[i] * y[i];
  }
  Var res = Var::sum(tem);

  x.insert(x.end(), y.begin(), y.end());   // x is now used as inputs sequence
  res.reorderInput(x);
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    double ans = 0;
    for (int i = 0; i < 25; i++)
      ans += ins[i] * ins[i + 25];
    return ans;
  });

  auto result = res.simulate();
  res.config_by_str(result.second);
  res.genVerilog();
}