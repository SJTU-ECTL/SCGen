#include "Modules/Var.h"

using namespace SC_sim;

void NN_4(const Config &conf) {
  std::vector<Var> x(4);
  std::vector<Var> y(4);
  std::vector<double> para = {0.0625, 0.125, 0.0625, 0.125};
  std::vector<Var> tem(4);
  std::vector<GiNaC::symbol> symbols(4);
  for (int i = 0; i < 4; i++)
    symbols[i] = GiNaC::symbol("x_" + std::to_string(i));

  for (int i = 0; i < 4; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG, nullptr, symbols[i]);
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

  res.expr();

  auto possible_cliques = res.detect_CI(1);
  auto result = res.simulate(possible_cliques);

  res.config_by_str(result.second);
  res.genVerilog();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 10;
  conf.graph_name = "NN_4_shared";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.rand_till_converge = false;
  conf.seed = rand();

  conf.scType = bipolar;

  conf.bit_width = 8;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  NN_4(conf);
}