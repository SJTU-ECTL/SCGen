#ifndef STOCHASTIC_COMPUTING_UTILS_IO_H
#define STOCHASTIC_COMPUTING_UTILS_IO_H

#include <iostream>
#include <sstream>
#include <vector>

/**
 * Print vector contents to stream, without new line at the end.
 * @tparam T
 * @param stream
 * @param vec
 */
template <typename T>
void print_vector(std::ostream &stream, const std::vector<T> &vec) {
  int len = vec.size();
  for (int i = 0; i < len - 1; i++) {
    stream << vec[i] << ' ';
  }
  if (len > 0)
    stream << vec[len - 1];
}

template <typename T> std::string array_str(const std::vector<T> &vec) {
  std::stringstream str;
  str << "[";
  int len = vec.size();
  for (int i = 0; i < len - 1; i++) {
    str << vec[i] << ", ";
  }
  if (len > 0)
    str << vec[len - 1];
  str << "]";
  return str.str();
}

std::vector<int> parse_str_array(const std::string &str);

#endif // STOCHASTIC_COMPUTING_UTILS_IO_H
