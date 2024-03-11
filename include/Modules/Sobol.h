#ifndef STOCHASTIC_COMPUTING_SOBOL_H
#define STOCHASTIC_COMPUTING_SOBOL_H

#include "Headers.h"
#include "Utils/Utils_IO.h"

class Sobol {
private:
  void generate_pivots_helper(const int &n, const int &index,
                              bitstream_list cur_list,
                              std::vector<bitstream_list> &ans);
  std::vector<bitstream_list> generate_pivots(const int &n);
  static int get_num(const bitstream_list &binary,
                     const bitstream_list &scrambling);
  /**
   *
   * @param n
   * @param cur be updated
   * @param poly
   * @param inverter
   */
  static void process(const int &n, bitstream_list &cur,
                      const bitstream_list &poly,
                      const bitstream_list &inverter);

public:
  class setting {
  public:
    bitstream_list seed;
    bitstream_list polynomial;
    bitstream_list scrambling;
    bitstream_list inverter;
    bool inserting_zero;
    int zero_position;
    setting(bitstream_list seed, bitstream_list poly = {},
            bitstream_list scram = {}, bitstream_list inverter = {},
            bool inserting_zero = true, int zero_position = 0) {
      this->seed = std::move(seed);
      this->polynomial = std::move(poly);
      this->scrambling = std::move(scram);
      this->inverter = std::move(inverter);
      this->inserting_zero = inserting_zero;
      this->zero_position = zero_position;
    }
  };
  int N;
  Sobol(int N_);

  bitstream_list scrambling(const bitstream_list &output,
                            const bitstream_list &scram) const;

  bitstream_list simulate(const setting &set) const;
  std::vector<bitstream_list> search_polynomials();

  std::vector<int> get_DV(const setting &set) const;

  std::vector<std::vector<int>> generate_seeds();
  std::vector<int> random_seed();
};

#endif   // STOCHASTIC_COMPUTING_SOBOL_H
