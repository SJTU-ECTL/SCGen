#include "Modules/Var.h"
#include <string>

using namespace SC_sim;

/**
  For single LFSR, the generation code of LFSR is in
  `VerilogSCWrapperGenerator.m`.

  The seed is determined in `VerilogSCWrapperGenerator.m`.
  The polynomial is determined in `VerilogLFSRGenerator.m`.

  Conversion is needed for polynomial configuration:
    Removing the first number in scsynth polynomial
    For next numbers: SCGen_num = scsynth_num - 1
*/

std::string get_scsynth_config(int bitwidth) {
  // clang-format off
  switch (bitwidth) {
  case 6:
    return "config of scsynth of bit 6\n"
           "1: node id: 1, polynomial: [4], seed: [0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "2: node id: 5, polynomial: [4], seed: [0, 0, 0, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "3: node id: 12, polynomial: [4], seed: [0, 0, 0, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "4: node id: 16, polynomial: [4], seed: [0, 0, 0, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "5: node id: 23, polynomial: [4], seed: [0, 0, 0, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "6: node id: 27, polynomial: [4], seed: [0, 0, 1, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "7: node id: 34, polynomial: [4], seed: [0, 0, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "8: node id: 38, polynomial: [4], seed: [0, 0, 1, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "9: node id: 45, polynomial: [4], seed: [0, 0, 1, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "10: node id: 49, polynomial: [4], seed: [0, 1, 0, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "11: node id: 56, polynomial: [4], seed: [0, 1, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "12: node id: 60, polynomial: [4], seed: [0, 1, 0, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "13: node id: 67, polynomial: [4], seed: [0, 1, 0, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "14: node id: 71, polynomial: [4], seed: [0, 1, 0, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "15: node id: 78, polynomial: [4], seed: [0, 1, 1, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "16: node id: 82, polynomial: [4], seed: [0, 1, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "17: node id: 89, polynomial: [4], seed: [0, 1, 1, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n"
           "18: node id: 93, polynomial: [4], seed: [0, 1, 1, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5], leq: 0 inserting_zero: 0\n";

  case 7:
    return "config of scsynth of bit 7\n"
           "1: node id: 1, polynomial: [5], seed: [0, 0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "2: node id: 5, polynomial: [5], seed: [0, 0, 0, 0, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "3: node id: 12, polynomial: [5], seed: [0, 0, 0, 0, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "4: node id: 16, polynomial: [5], seed: [0, 0, 0, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "5: node id: 23, polynomial: [5], seed: [0, 0, 0, 1, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "6: node id: 27, polynomial: [5], seed: [0, 0, 1, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "7: node id: 34, polynomial: [5], seed: [0, 0, 1, 0, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "8: node id: 38, polynomial: [5], seed: [0, 0, 1, 1, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "9: node id: 45, polynomial: [5], seed: [0, 0, 1, 1, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "10: node id: 49, polynomial: [5], seed: [0, 0, 1, 1, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "11: node id: 56, polynomial: [5], seed: [0, 1, 0, 0, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "12: node id: 60, polynomial: [5], seed: [0, 1, 0, 0, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "13: node id: 67, polynomial: [5], seed: [0, 1, 0, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "14: node id: 71, polynomial: [5], seed: [0, 1, 0, 1, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "15: node id: 78, polynomial: [5], seed: [0, 1, 1, 0, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "16: node id: 82, polynomial: [5], seed: [0, 1, 1, 0, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "17: node id: 89, polynomial: [5], seed: [0, 1, 1, 0, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n"
           "18: node id: 93, polynomial: [5], seed: [0, 1, 1, 1, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6], leq: 0 inserting_zero: 0\n";

  case 8:
    return "config of scsynth of bit 8\n"
           "1: node id: 1, polynomial: [0, 5, 6], seed: [0, 0, 0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "2: node id: 5, polynomial: [0, 5, 6], seed: [0, 0, 0, 0, 0, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "3: node id: 12, polynomial: [0, 5, 6], seed: [0, 0, 0, 0, 1, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "4: node id: 16, polynomial: [0, 5, 6], seed: [0, 0, 0, 1, 0, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "5: node id: 23, polynomial: [0, 5, 6], seed: [0, 0, 0, 1, 1, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "6: node id: 27, polynomial: [0, 5, 6], seed: [0, 0, 1, 0, 0, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "7: node id: 34, polynomial: [0, 5, 6], seed: [0, 0, 1, 0, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "8: node id: 38, polynomial: [0, 5, 6], seed: [0, 0, 1, 1, 0, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "9: node id: 45, polynomial: [0, 5, 6], seed: [0, 0, 1, 1, 0, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "10: node id: 49, polynomial: [0, 5, 6], seed: [0, 0, 1, 1, 1, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "11: node id: 56, polynomial: [0, 5, 6], seed: [0, 1, 0, 0, 0, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "12: node id: 60, polynomial: [0, 5, 6], seed: [0, 1, 0, 0, 1, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "13: node id: 67, polynomial: [0, 5, 6], seed: [0, 1, 0, 1, 0, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "14: node id: 71, polynomial: [0, 5, 6], seed: [0, 1, 0, 1, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "15: node id: 78, polynomial: [0, 5, 6], seed: [0, 1, 1, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "16: node id: 82, polynomial: [0, 5, 6], seed: [0, 1, 1, 0, 1, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "17: node id: 89, polynomial: [0, 5, 6], seed: [0, 1, 1, 0, 1, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n"
           "18: node id: 93, polynomial: [0, 5, 6], seed: [0, 1, 1, 1, 0, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7], leq: 0 inserting_zero: 0\n";

  case 9:
    return "config of scsynth of bit 9\n"
           "1: node id: 1, polynomial: [4], seed: [0, 0, 0, 0, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "2: node id: 5, polynomial: [4], seed: [0, 0, 0, 0, 0, 1, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "3: node id: 12, polynomial: [4], seed: [0, 0, 0, 0, 1, 1, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "4: node id: 16, polynomial: [4], seed: [0, 0, 0, 1, 0, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "5: node id: 23, polynomial: [4], seed: [0, 0, 0, 1, 1, 0, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "6: node id: 27, polynomial: [4], seed: [0, 0, 1, 0, 0, 0, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "7: node id: 34, polynomial: [4], seed: [0, 0, 1, 0, 1, 0, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "8: node id: 38, polynomial: [4], seed: [0, 0, 1, 1, 0, 0, 0, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "9: node id: 45, polynomial: [4], seed: [0, 0, 1, 1, 0, 1, 1, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "10: node id: 49, polynomial: [4], seed: [0, 0, 1, 1, 1, 1, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "11: node id: 56, polynomial: [4], seed: [0, 1, 0, 0, 0, 1, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "12: node id: 60, polynomial: [4], seed: [0, 1, 0, 0, 1, 1, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "13: node id: 67, polynomial: [4], seed: [0, 1, 0, 1, 0, 0, 1, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "14: node id: 71, polynomial: [4], seed: [0, 1, 0, 1, 1, 0, 1, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "15: node id: 78, polynomial: [4], seed: [0, 1, 1, 0, 0, 0, 0, 1, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "16: node id: 82, polynomial: [4], seed: [0, 1, 1, 0, 1, 0, 0, 0, 0], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "17: node id: 89, polynomial: [4], seed: [0, 1, 1, 0, 1, 1, 1, 0, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n"
           "18: node id: 93, polynomial: [4], seed: [0, 1, 1, 1, 0, 1, 0, 1, 1], scrambling: [0, 1, 2, 3, 4, 5, 6, 7, 8], leq: 0 inserting_zero: 0\n";

  }
  return "";
  // clang-format on
}

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

  std::string config = get_scsynth_config(conf.bit_width);
  auto results = res.simulate_conf(config);
  std::cout << results.second << std::endl;
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
  conf.optimize = 0;
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