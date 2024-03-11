#include "Utils.h"

void scrambling_helper(const int &n, const int &index,
                       std::vector<bitstream_list> nows,
                       std::vector<bitstream_list> &answer) {
  if (index == n) {
    answer = std::move(nows);
    return;
  }

  std::vector<bitstream_list> nexts;
  for (const auto &now : nows) {
    for (int i = 0; i <= now.size(); i++) {
      bitstream_list temp(now);
      temp.insert(temp.begin() + i, index);
      nexts.push_back(std::move(temp));
    }
  }
  scrambling_helper(n, index + 1, nexts, answer);
}

std::vector<bitstream_list> generate_scrambling(const int &n) {
  std::vector<bitstream_list> ans;
  scrambling_helper(n, 1, {{0}}, ans);
  return ans;
}

bitstream_list num_to_arr(const int &n, int num) {
  bitstream_list ans(n);
  for (int i = n - 1; i >= 0; i--) {
    ans[i] = num % 2;
    num /= 2;
  }
  return ans;
}

std::vector<bitstream_list> generate_seeding(const int &n) {
  std::vector<bitstream_list> ans;
  int LEN = (int)pow(2, n);
  for (int i = 1; i < LEN; i++) {
    ans.push_back(num_to_arr(n, i));
  }
  return ans;
}

std::vector<bitstream_list> nums_to_bit_arrays(const int &n,
                                               const bitstream_list &nums) {
  std::vector<bitstream_list> ans;
  for (const auto &num : nums)
    ans.push_back(num_to_arr(n, num));
  return ans;
}

int arr_to_num(const bitstream_list &arr) {
	// largest i=0; smallest i = len-1
  int len = (int)arr.size();
  int multiplier = 1;
  int result = 0;
  for (int i = len - 1; i >= 0; i--) {
    result += multiplier * arr[i];
    multiplier <<= 1;
  }
  return result;
}

int arr_to_signed_num(bitstream_list arr){
	int len = (int)arr.size();
	if (arr[0] == 1) // neg
	{
		for (int i = len - 1; i >= 0; i--) {
			arr[i] = arr[i] == 1? 0:1;
		}
		return -(arr_to_num(arr) + 1);
	}
	// positive

	return arr_to_num(arr);
}


bitstream_list
bit_arrays_to_nums(const int &n,
                   const std::vector<bitstream_list> &bit_arrays) {
  bitstream_list ans;
  for (const auto &array : bit_arrays)
    ans.push_back(arr_to_num(array));
  return ans;
}

bitstream_list comparator(const bitstream_list &arr, const int &num,
                          const bool &le, bool is_signed,int bit_width) {
  bitstream_list ret(arr);
  if (le) {
    for (auto &r : ret) {
    	if (is_signed){
				int signed_r =  2*arr_to_signed_num(num_to_arr(bit_width,r));
				if (signed_r <= num)
					r = 1;
				else
					r = 0;
    	} else {
				if (r <= num)
					r = 1;
				else
					r = 0;
    	}
    }
  } else {
    for (auto &r : ret) {
			if (is_signed){
				int signed_r = 2*arr_to_signed_num(num_to_arr(bit_width,r));
				//std::cout<<"num: "<< num<<" | lfsr: "<< signed_r <<std::endl;
				if (signed_r < num)
					r = 1;
				else
					r = 0;
			} else {
				if (r < num)
					r = 1;
				else
					r = 0;
			}
    }
  }
  return ret;
}

/**
 *
 * @param arr
 * @param start inclusive
 * @param end exclusive
 * @return
 */
int count_arr(const bitstream_list &arr, const size_t &start,
              const size_t &end) {
  int num = 0;
  //    for (const auto &a : arr) {
  //        if (a == 1)
  //            num++;
  //    }
  for (size_t i = start; i < end; i++) {
    if (arr[i] == 1)
      num++;
  }
  return num;
}

int count_arr_1_0(const bitstream_list &arr, const size_t &start,
							const size_t &end) {
	int num = 0;

	for (size_t i = start; i < end; i++) {
		if (arr[i] == 1)
			num++;
		else
			num--;
	}
	return num;
}

bitstream_list DFF(const bitstream_list &arr, const int &num) {
  bitstream_list ans(arr.begin() + num, arr.end());
  ans.insert(ans.end(), arr.begin(), arr.begin() + num);
  return ans;
}

/*
int main() {
//    print_vector(std::cout, num_to_arr(5, 7));
//    list arr({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
//    print_vector(std::cout, DFF(arr, 2));
    std::vector<list> ret = generate_scrambling(4);
    for (const auto &r : ret) {
        for (const auto &s : r) {
            std::cout << s << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << ret.size() << std::endl;
}*/

void trim(std::string &line) {
  if (line.size() == 0)
    return;
  int end = 0;
  int len = line.size();
  while (end < len && line[end] == ' ') {
    end++;
  }
  line = line.substr(end, line.size() - end);
  // trim right
  end = line.size() - 1;
  while (end >= 0 && line[end] == ' ') {
    end--;
  }
  line = line.substr(0, end + 1);
}

int fall_in_interval(int index, const bitstream_list &input_lfsr_num){
    for (int i = input_lfsr_num.size()-1;i>=1;i--){
        if ((index < input_lfsr_num[i]) && (index >= input_lfsr_num[i-1])){
            return i;
        }
    }
    return 0;
}