#include "Modules/Var.h"
// #include <compare>
#include <ginac/symbol.h>
#include <string>

void x2(Config conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::SNG_TYPE::LFSR_SNG, NULL,
                GiNaC::symbol("x"));

  SC_sim::Var res = x * SC_sim::Op::DFF(x, 3);

  res.reorderInput({x});

  res.addCalculate(
      "MAE", [](const std::vector<double> &ins) { return ins[0] * ins[0]; });

  res.expr();
  auto results = res.simulate();
  res.config_by_str(results.second);
  res.genVerilog();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1000000;
  conf.simType = full_sim;
  conf.graph_name = "x2_DFF";
  conf.optimize = 0;
  conf.seed = rand();

  conf.search_method = "RANDOM";
  conf.bit_width = 6;
  if (argc > 1)
    conf.search_method = argv[1];
  if (argc > 2)
    conf.bit_width = std::stoi(argv[2]);

  x2(conf);
}
