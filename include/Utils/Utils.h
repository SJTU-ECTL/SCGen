#ifndef STOCHASTIC_COMPUTING_UTILS_H
#define STOCHASTIC_COMPUTING_UTILS_H

#include "Headers.h"
#include "Utils_IO.h"

void scrambling_helper(const int &n, const int &index,
                       std::vector<bitstream_list> nows,
                       std::vector<bitstream_list> &answer);

std::vector<bitstream_list> generate_scrambling(const int &n);

bitstream_list num_to_arr(const int &n, int num);

std::vector<bitstream_list> generate_seeding(const int &n);

std::vector<bitstream_list> nums_to_bit_arrays(const int &n,
                                               const bitstream_list &nums);

int arr_to_num(const bitstream_list &arr);

bitstream_list
bit_arrays_to_nums(const int &n, const std::vector<bitstream_list> &bit_arrays);

bitstream_list comparator(const bitstream_list &arr, const int &num,
                          const bool &le = true, bool is_signed = false, int bit_width = 6);

int count_arr(const bitstream_list &arr, const size_t &start,
              const size_t &end);

int count_arr_1_0(const bitstream_list &arr, const size_t &start,
							const size_t &end);

bitstream_list DFF(const bitstream_list &arr, const int &num);

void trim(std::string &line);

int fall_in_interval(int index, const bitstream_list &inputs_num);

#endif // STOCHASTIC_COMPUTING_UTILS_H
