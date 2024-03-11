#include "Modules/Var.h"
#include <ginac/symbol.h>

void x_plus_y(const auto &conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::SOBOL_SNG, NULL, GiNaC::symbol("x"));
  SC_sim::Var y(SC_sim::SC, conf, SC_sim::SOBOL_SNG, NULL, GiNaC::symbol("y"));

  SC_sim::Var res = x + y;

  res.reorderInput({x, y});
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return (ins[0] + ins[1]) / 2;
  });

  res.expr();

  std::vector<std::vector<std::vector<Node *>>> possible_cliques =
      res.detect_CI(1);
  auto result = res.simulate(possible_cliques);

  res.config_by_str(result.second);
  res.genVerilog();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 10;
  conf.graph_name = "x_plus_y_shared";
  conf.simType = full_sim;
  conf.rand_till_converge = false;
  conf.seed = rand();

  conf.bit_width = 8;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  x_plus_y(conf);
}
