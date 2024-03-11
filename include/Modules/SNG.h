//
// Created by 李泽玺 on 2022/3/7.
//

#ifndef STOCHASTIC_COMPUTING_SNG_H
#define STOCHASTIC_COMPUTING_SNG_H

#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <vector>

enum SNG_t { LFSR_t, BINOMIAL_t, HYPERGEOMETRIC_t, SOBOL_t };

class SNG {
  //    SNG_t type;
  //    // bit width
  //    int n;
public:
  //    SNG(SNG_t _type, int _n) : type(_type), n(_n) {
  //    }

  // LEN = 2^n - 1 if input 0 and inserting_zero is false
  // LEN = 2^n if input 0 and inserting_zero is true
  static std::vector<int> simulate(SNG_t type, int n, int LEN = 0,
                                   bool inserting_zero = false) {
    int NN = (int)std::pow(2, n);
    if (LEN == 0 && inserting_zero)
      LEN = NN;
    else if (LEN == 0 && !inserting_zero)
      LEN = NN - 1;

    //        srand(time(NULL));
    std::vector<int> ans(LEN, 0);
    switch (type) {
    case LFSR_t:
      break;
    case BINOMIAL_t:
      for (int i = 0; i < LEN; i++) {
        if (inserting_zero) {
          // random number in range [0, 2^n - 1]
          ans[i] = rand() % NN;
        } else {
          // random number in range [1, 2^n - 1]
          ans[i] = (rand() % (NN - 1)) + 1;
        }
      }
      break;
    case HYPERGEOMETRIC_t:
      for (int i = 0; i < LEN; i++) {
        if (inserting_zero)
          ans[i] = i;
        else
          ans[i] = i + 1;
      }
      // shuffle
      for (int i = 0; i < LEN - 1; i++) {
        int j = i + (rand() % (LEN - i));
        std::swap(ans[i], ans[j]);
      }
      break;
    case SOBOL_t:
      break;
    default:
      break;
    }

    return ans;
  }
};

#endif // STOCHASTIC_COMPUTING_SNG_H
