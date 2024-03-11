#include "Modules/Var.h"
#include <ginac/symbol.h>
#include <string>

using namespace SC_sim;

void gaussian_blur(const Config &conf) {
  std::vector<Var> x(9);
  std::vector<Var> y(9);
  std::vector<double> para = {0.0625, 0.125,  0.0625, 0.125, 0.25,
                              0.125,  0.0625, 0.125,  0.0625};
  std::vector<Var> tem(9);
  std::vector<GiNaC::symbol> symbols(9);
  for (int i = 0; i < 9; i++)
    symbols[i] =
        GiNaC::symbol("x_" + std::to_string(i / 3) + std::to_string(i % 3));

  for (int i = 0; i < 9; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG, nullptr, symbols[i]);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf, LFSR_SNG, nullptr);
    tem[i] = x[i] * y[i];
  }
  Var res = Var::sum(tem);

  x.insert(x.end(), y.begin(), y.end());
  res.reorderInput(x);
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return (ins[0] * ins[9] + ins[1] * ins[10] + ins[2] * ins[11] +
            ins[3] * ins[12] + ins[4] * ins[13] + ins[5] * ins[14] +
            ins[6] * ins[15] + ins[7] * ins[16] + ins[8] * ins[17]);
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
  conf.graph_name = "GB_3x3_shared";
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
