#include "Modules/Var.h"

void xy(Config conf) {
  SC_sim::Var x(SC_sim::SC, conf, SC_sim::SNG_TYPE::LFSR_SNG, NULL,
                GiNaC::symbol("x"));
  SC_sim::Var y(SC_sim::SC, conf, SC_sim::SNG_TYPE::LFSR_SNG, NULL,
                GiNaC::symbol("y"));

  SC_sim::Var res = x * y;

  res.addCalculate(
      "MAE", [](const std::vector<double> &ins) { return ins[0] * ins[1]; });

  res.simulate();
}

int main() {
  Config conf;
  conf.inserting_zero = 0;
  conf.leq = 0;
  conf.bit_width = 6;
  conf.rand_times = 10000;
  conf.graph_name = "xy";
  conf.optimize = 0;
  conf.seed = rand();
  conf.rand_till_converge = false;

  int bit_widths[4] = {6, 7, 8, 9};
  for (auto bit_width : bit_widths) {
    conf.bit_width = bit_width;

    conf.optimize = 0;
    xy(conf);

    conf.optimize = 1;
    xy(conf);

    conf.optimize = 4;
    xy(conf);

    conf.optimize = 5;
    xy(conf);
  }
}