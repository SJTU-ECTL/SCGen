#include "Modules/Var.h"
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
    symbols[i] = GiNaC::symbol("x_{" + std::to_string(i / 3) + "," +
                               std::to_string(i % 3) + "}");

  for (int i = 0; i < 9; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG, NULL, symbols[i]);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf);
    tem[i] = Op::LUT((x[i] * y[i]).to_BC());
  }

  Var res = Var::sum(tem);
  res.reorderInput({x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], y[0],
                    y[1], y[2], y[3], y[4], y[5], y[6], y[7], y[8]});
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return (ins[0] * ins[9] + ins[1] * ins[10] + ins[2] * ins[11] +
            ins[3] * ins[12] + ins[4] * ins[13] + ins[5] * ins[14] +
            ins[6] * ins[15] + ins[7] * ins[16] + ins[8] * ins[17]);
  });

  res.simulate();
}

int main(int argc, char *argv[]) {
  int mult = 10;

  Config conf;
  conf.inserting_zero = 0;
  conf.leq = 0;
  conf.bit_width = 6;
  conf.rand_times = 1000;
  conf.graph_name = "gaussian_blur";
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

  int bit_widths[4] = {6, 7, 8, 9};
  for (auto bit_width : bit_widths) {
    conf.bit_width = bit_width;

    conf.optimize = 0;
    gaussian_blur(conf);

    conf.optimize = 2;
    gaussian_blur(conf);

    conf.optimize = 4;
    gaussian_blur(conf);

    conf.optimize = 6;
    gaussian_blur(conf);
  }
}