#include "LFSR.h"
#include "Utils/Utils.h"

void LFSR::generate_pivots_helper(const int &n, const int &index,
                                  bitstream_list cur_list,
                                  std::vector<bitstream_list> &ans) {
  if (index == n - 1) {
    ans.push_back(cur_list);
    return;
  }
  cur_list.push_back(index);
  generate_pivots_helper(n, index + 1, cur_list, ans);
  cur_list.pop_back();
  generate_pivots_helper(n, index + 1, cur_list, ans);
}
std::vector<bitstream_list> LFSR::generate_pivots(const int &n) {
  std::vector<bitstream_list> ans;
  generate_pivots_helper(n, 0, bitstream_list(), ans);
  return ans;
}

int LFSR::get_num(const bitstream_list &binary,
                  const bitstream_list &scrambling) {
  int len = (int)binary.size();
  int multiplier = 1;
  int result = 0;
  if (scrambling.empty()) {
    for (int i = len - 1; i >= 0; i--) {
      result += multiplier * binary[i];
      multiplier <<= 1;
    }
  } else {
    for (int i = len - 1; i >= 0; i--) {
      result += multiplier * binary[scrambling[i]];
      multiplier <<= 1;
    }
  }
  return result;
}
void LFSR::process(const int &n, bitstream_list &cur,
                   const bitstream_list &poly, const bitstream_list &inverter) {
  int node = cur[cur.size() - 1];
  if (!inverter.empty()) {
    for (int i = 0; i < inverter.size(); i++) {
      if (inverter[i] == 1)
        cur[i] ^= 1;
    }
  }
  for (const auto &p : poly)
    node ^= cur[p];
  cur.insert(cur.begin(), node);
  cur.pop_back();
}

LFSR::LFSR_(int N_, bitstream_list LFSR_inverter_) {
  N = N_;
  LFSR_inverter = std::move(LFSR_inverter_);
}
[[nodiscard]] bitstream_list LFSR::simulate(const setting &set) const {
  bitstream_list output;
  std::unordered_set<int> map;
  bitstream_list cur(set.seed);

  int index = 0;
  int before_zero = -1;
  if (set.inserting_zero) {
    if (set.inverter.empty()) {
      bitstream_list Temp(set.seed.size());
      Temp[set.seed.size() - 1] = 1;
      before_zero = get_num(Temp, set.scrambling);
    } else {
      bitstream_list Temp = set.inverter;
      Temp[Temp.size() - 1] ^= 1;
      before_zero = get_num(Temp, set.scrambling);
    }
  }

  while (true) {
    int num = get_num(cur, set.scrambling);
    if (map.contains(num))
      break;
    map.insert(num);
    output.push_back(num);
    if (set.inserting_zero && num == before_zero) {
      // Not consider inverter now.
      output.push_back(0);
      map.insert(0);
    }
    process(N, cur, set.polynomial, set.inverter);

    index++;
  }
  return output;
}
std::vector<bitstream_list> LFSR::search_polynomials() {
  std::vector<bitstream_list> pivots = generate_pivots(N);
  pivots.pop_back();

  int LEN = (int)pow(2, N);

  bitstream_list seed;
  seed.assign(N, 0);
  seed[N - 1] = 1;

  std::vector<bitstream_list> polynomials;
  for (auto &poly : pivots) {
    LFSR_::setting set(seed, poly, {}, {}, false, 0);
    bitstream_list nums = simulate(set);
    if (nums.size() == LEN - 1)
      polynomials.push_back(poly);
  }
  return polynomials;
}

// int main() {
//     LFSR a(8);
//     std::vector<list> polys = a.search_polynomials();
//     for (auto &p : polys) {
//         for (auto &q : p)
//             std::cout << q << ' ';
//         std::cout << std::endl;
//     }
//     std::cout << polys.size() << std::endl;
// }