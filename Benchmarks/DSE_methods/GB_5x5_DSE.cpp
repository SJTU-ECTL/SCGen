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
    symbols[i] = GiNaC::symbol("x_{" + std::to_string(i / dim) + "," +
                               std::to_string(i % dim) + "}");

  for (int i = 0; i < dim * dim; i++) {
    x[i] = Var(SC_sim::SC, conf, LFSR_SNG, NULL, symbols[i]);
    y[i] = Var::constantVar(para[i], SC_sim::SC, conf);
    tem[i] = Op::LUT((x[i] * y[i]).to_BC());
  }
  Var res = Var::sum(tem);

  x.insert(x.end(), y.begin(), y.end());   // x is now used as inputs sequence
  res.reorderInput(x);
  res.addCalculate("MAE", [](const std::vector<double> &ins) {
    return ins[0] * ins[25] + ins[1] * ins[26] + ins[2] * ins[27] +
           ins[3] * ins[28] + ins[4] * ins[29] + ins[5] * ins[30] +
           ins[6] * ins[31] + ins[7] * ins[32] + ins[8] * ins[33] +
           ins[9] * ins[34] + ins[10] * ins[35] + ins[11] * ins[36] +
           ins[12] * ins[37] + ins[13] * ins[38] + ins[14] * ins[39] +
           ins[15] * ins[40] + ins[16] * ins[41] + ins[17] * ins[42] +
           ins[18] * ins[43] + ins[19] * ins[44] + ins[20] * ins[45] +
           ins[21] * ins[46] + ins[22] * ins[47] + ins[23] * ins[48] +
           ins[24] * ins[49];
  });

  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1000;
  conf.graph_name = "GB_5x5";
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

  gaussian_blur(conf);
}