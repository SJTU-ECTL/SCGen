#include "Utils_IO.h"

std::vector<int> parse_str_array(const std::string &str) {
  std::vector<int> ans;
  std::istringstream out(str);
  char temp;
  // temp << out;
  out >> temp;   // [
  int temp_i;
  while (temp != ']') {
    out >> temp_i;
    ans.push_back(temp_i);
    out >> temp;
  }
  out >> temp;   // ]
  // print_vector(std::cout, ans);
  return ans;
}

/*
template<typename T>
void print_vector(std::ostream& stream, const std::vector<T> &vec) {
    int len = vec.size();
    for (int i = 0; i < len - 1; i++) {
        stream << vec[i] << ' ';
    }
    stream << vec[len - 1];
}
*/
