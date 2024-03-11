#include "Modules/Var.h"

using namespace SC_sim;

void NN_4(const Config &conf) {
  std::vector<Var> x(4);
  std::vector<Var> y(4);
  std::vector<double> para = {0.0625, 0.125, 0.0625, 0.125};
  std::vector<Var> tem(4);
  std::vector<GiNaC::symbol> symbols(4);
  for (int i = 0; i < 4; i++)
    symbols[i] = GiNaC::symbol("x_{" + std::to_string(i / 4) + "," +
                               std::to_string(i % 4) + "}");

  for (int i = 0; i < 4; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG, NULL, symbols[i]);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf);
    tem[i] = Op::LUT((x[i] * y[i]).to_BC());
  }

  Var res = Op::TANH(Var::Randomizer(Var::sum(tem)), 2);

  x.insert(x.end(), y.begin(), y.end());   // x is now used as inputs sequence
  res.reorderInput(x);

  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return tanh(2 * (ins[0] * ins[4] + ins[1] * ins[5] + ins[2] * ins[6] +
                     ins[3] * ins[7]));
  });

  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.inserting_zero = 0;
  conf.leq = 0;
  conf.bit_width = 6;
  conf.rand_times = 1000;
  conf.graph_name = "NN_4";
  conf.optimize = 0;
  conf.seed = rand();
  conf.simType = rand_sim;
  conf.rand_sim_times = 100000;
  conf.rand_till_converge = false;

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.rand_till_converge = std::string(argv[2]).compare("false");
  } else
    conf.search_method = "RANDOM";

  conf.scType = bipolar;

  int bit_widths[4] = {6, 7, 8, 9};
  for (auto bit_width : bit_widths) {
    conf.bit_width = bit_width;

    conf.optimize = 0;
    NN_4(conf);

    conf.optimize = 2;
    NN_4(conf);

    conf.optimize = 4;
    NN_4(conf);

    conf.optimize = 6;
    NN_4(conf);
  }
}