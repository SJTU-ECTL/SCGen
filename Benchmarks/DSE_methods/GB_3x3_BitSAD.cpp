#include "Modules/Var.h"
#include <string>

using namespace SC_sim;

/**
  BitSAD v2 only generates fixed design of LFSR SNG at 20 and 32 bit widths.
  Here we generate randomly configured LFSRs instead.
*/

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
  Config conf;
  conf.rand_times = 1;
  conf.graph_name = "GB_3x3";
  conf.simType = rand_sim;
  conf.rand_sim_times = 10000;
  conf.rand_till_converge = false;
  conf.seed = rand();

  conf.bit_width = 6;
  conf.optimize = 6;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  std::vector<int> bit_widths = {6, 7, 8, 9};
  for (int bit_width : bit_widths) {
    conf.bit_width = bit_width;
    gaussian_blur(conf);
  }
}