#include "SC_core.h"
#include "Utils/Utils.h"

using namespace std;

int SC_core::simulate(vector<int> inputs) { return real_fn(inputs); }

bitstream_list SC_core::simulate_list(vector<bitstream_list> input_lists) {
  size_t number = input_lists.size();
  bitstream_list ans(LEN, 0);
  for (int i = 0; i < LEN; i++) {
    vector<int> inputs(number);
    for (int j = 0; j < number; j++)
      inputs[j] = input_lists[j][i];
    ans[i] = simulate(inputs);
  }
  return ans;
}

double SC_core::simulate_list_value(vector<bitstream_list> input_lists) {
  bitstream_list outputs = simulate_list(input_lists);
  return (double)arr_to_num(outputs) / LEN;
}

double SC_core::simulate_MAE(vector<bitstream_list> input_lists,
                             vector<int> input_values) {
  return abs(simulate_list_value(input_lists) - real_fn(input_values));
}
