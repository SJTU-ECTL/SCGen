#include "Gates.h"

bitstream_list NAND(const std::vector<bitstream_list> &inputs) {
  bitstream_list ans = AND(inputs);

  for (auto &a : ans)
    a ^= 1;

  return ans;
}

int NAND(const std::vector<int> &inputs) { return 1 - AND(inputs); }

int AND(const std::vector<int> &inputs) {
  int ans = 1;
  for (const auto &i : inputs) {
    ans *= i;
    if (ans == 0)
      break;
  }
  return ans;
}

bitstream_list AND(const std::vector<bitstream_list> &inputs) {
  size_t number = inputs.size();
  size_t len = inputs[0].size();
  bitstream_list ans(len, 1);

  for (size_t i = 0; i < number; i++) {
    for (size_t j = 0; j < len; j++) {
      ans[j] *= inputs[i][j];
    }
  }

  return ans;
}
