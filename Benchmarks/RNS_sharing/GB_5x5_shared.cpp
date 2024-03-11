#include "Modules/Var.h"
#include <string>

using namespace SC_sim;

void gaussian_blur(const Config &conf) {
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
  std::vector<GiNaC::symbol> symbols(dim * dim);
  for (int i = 0; i < dim * dim; i++)
    symbols[i] =
        GiNaC::symbol("x_" + std::to_string(i / dim) + std::to_string(i % dim));

  for (int i = 0; i < dim * dim; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG, nullptr, symbols[i]);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf, LFSR_SNG, nullptr);
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

  res.expr();

  auto possible_cliques = res.detect_CI(1);
  auto result = res.simulate(possible_cliques);

  res.config_by_str(result.second);
  res.genVerilog();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 10;
  conf.graph_name = "GB_5x5_shared";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.rand_till_converge = false;
  conf.seed = rand();

  conf.bit_width = 8;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  gaussian_blur(conf);
}