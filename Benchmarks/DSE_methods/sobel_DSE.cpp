#include "Modules/Var.h"
#include <ginac/symbol.h>

void sobel(const Config &conf) {
  SC_sim::Var x11(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x11"));
  SC_sim::Var x12(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x12"));
  SC_sim::Var x13(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x13"));
  SC_sim::Var x21(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x21"));
  SC_sim::Var x22(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x22"));
  SC_sim::Var x23(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x23"));
  SC_sim::Var x31(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x31"));
  SC_sim::Var x32(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x32"));
  SC_sim::Var x33(SC_sim::BC, conf, SC_sim::LFSR_SNG, NULL,
                  GiNaC::symbol("x33"));

  SC_sim::Var res1_ = (x13 + x23) + (x23 + x33);
  SC_sim::Var res2_ = (x11 + x21) + (x21 + x31);
  SC_sim::Var res3_ = (x11 + x12) + (x12 + x13);
  SC_sim::Var res4_ = (x31 + x32) + (x32 + x33);

  SC_sim::Var res1 = SC_sim::Var::Randomizer(res1_);
  SC_sim::Var res2 = SC_sim::Var::Randomizer(res2_, &res1);
  SC_sim::Var res3 = SC_sim::Var::Randomizer(res3_);
  SC_sim::Var res4 = SC_sim::Var::Randomizer(res4_, &res3);

  SC_sim::Var y1 = res1.abs_sub(res2);
  SC_sim::Var y2 = res3.abs_sub(res4);
  SC_sim::Var res = y1 + y2;

  res.reorderInput({x13, x23, x33, x11, x21, x31, x12, x32});
  res.addCalculate("min", [](const std::vector<double> &ins) {
    return (abs(((ins[0] + ins[1]) / 2 + (ins[1] + ins[2]) / 2) -
                ((ins[3] + ins[4]) / 2 + (ins[4] + ins[5]) / 2)) /
                2 +
            abs(((ins[3] + ins[6]) / 2 + (ins[6] + ins[0]) / 2) -
                ((ins[5] + ins[7]) / 2 + (ins[7] + ins[2]) / 2)) /
                2) /
           2;
  });

  res.simulate();
}

int main(int argc, char *argv[]) {
  Config conf;
  conf.rand_times = 1000;
  conf.graph_name = "sobel";
  conf.seed = rand();
  conf.simType = rand_sim;
  conf.rand_till_converge = false;
  conf.rand_sim_times = 1000;

  conf.bit_width = 6;
  conf.optimize = 0;
  conf.search_method = "RANDOM";

  if (argc > 1) {
    conf.search_method = argv[1];
    conf.bit_width = std::stoi(argv[2]);
  }

  sobel(conf);
}