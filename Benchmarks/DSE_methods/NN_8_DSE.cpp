#include "Modules/Var.h"

using namespace SC_sim;

void NN_8(const Config &conf) {
  std::vector<Var> x(8);
  std::vector<Var> y(8);
  std::vector<double> para = {0.025, 0.025, 0.15,  0.025,
                              0.15,  0.13,  0.125, 0.014};
  std::vector<Var> tem(8);
  std::vector<GiNaC::symbol> symbols(8);
  for (int i = 0; i < 8; i++)
    symbols[i] = GiNaC::symbol("x_{" + std::to_string(i / 4) + "," +
                               std::to_string(i % 4) + "}");

  for (int i = 0; i < 8; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG, NULL, symbols[i]);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf);
    tem[i] = Op::LUT((x[i] * y[i]).to_BC());
  }

  Var res = Op::TANH(Var::Randomizer(Var::sum(tem)), 2);

  x.insert(x.end(), y.begin(), y.end());   // x is now used as inputs sequence
  res.reorderInput(x);

  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return tanh(2 * (ins[0] * ins[8] + ins[1] * ins[9] + ins[2] * ins[10] +
                     ins[3] * ins[11] + ins[4] * ins[12] + ins[5] * ins[13] +
                     ins[6] * ins[14] + ins[7] * ins[15]));
  });
  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1000;
  conf.graph_name = "NN_8";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.rand_till_converge = false;
  conf.seed = rand();

  conf.scType = bipolar;

  conf.bit_width = 6;
  conf.optimize = 6;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  NN_8(conf);
}
