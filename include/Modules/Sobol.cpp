#include "Sobol.h"
#include "Headers.h"
#include "LFSR.h"
#include "Utils/Utils_IO.h"

Sobol::Sobol(int N_) { N = N_; }

std::vector<int> Sobol::get_DV(const setting &set) const {
  bitstream_list seed = set.seed;
  bitstream_list output;

  std::vector<int> Vs(N, 0);
  for (int i = 0; i < N; i++) {
    Vs[i] = (int) ((double) set.seed[i] / (1 << (i + 1)) * (1 << N));
  }
  // print_vector(std::cout, Vs);
  return Vs;
}

std::vector<bitstream_list> Sobol::search_polynomials() {
  LFSR lfsr(this->N);

  std::vector<bitstream_list> polynomials = lfsr.search_polynomials();
  return polynomials;
}

int LSZ(int number) {
  int t = 1;
  int i = 0;
  while (((number & t) ^ t) == 0) {
    t <<= 1;
    i += 1;
  }
  return i;
}

bitstream_list Sobol::scrambling(const bitstream_list &output,
                                 const bitstream_list &scram) const {
  bitstream_list new_output;
  // print_vector(std::cout, scram);
  for (int num : output) {
    int new_num = 0;
    int multiplier = 1;
    int tmp = 0;
    for (int i = scram.size() - 1; i >= 0; i--) {
      tmp = this->N - 1 - scram[i];
      new_num += ((num & (1 << tmp)) >> tmp) * multiplier;
      multiplier <<= 1;
    }
    new_output.push_back(new_num);
  }
  return new_output;
}

bitstream_list Sobol::simulate(const setting &set) const {
  bitstream_list seed = set.seed;
  bitstream_list output;

  int LEN = (int) pow(2, N);

  std::vector<int> Vs(N, 0);
  for (int i = 0; i < N; i++) {
    Vs[i] = (int) ((double) set.seed[i] / (1 << (i + 1)) * (1 << N));
  }

  output = std::vector<int>(LEN, 0);
  for (int i = 0; i < LEN - 1; i++) {
    int k = LSZ(i);
    output[i + 1] = output[i] ^ Vs[k];
  }

  // Scrambling
  output = scrambling(output, set.scrambling);

  return output;
}

std::vector<int> generate_seeds_helper(int index, int N) {
  if (index >= N)
    return {};

  std::vector<int> possible_choice;
  for (int i = 1; i < 1 << (index + 1); i += 2)
    possible_choice.push_back(i);
  return possible_choice;
}

std::vector<std::vector<int>> Sobol::generate_seeds() {
  std::vector<std::vector<int>> seeds(1, std::vector<int>(1, 1));

  for (int i = 1; i < N; i++) {
    std::vector<int> possible_choice = generate_seeds_helper(i, this->N);
    std::vector<std::vector<int>> temp;
    for (auto &seed : seeds) {
      for (const auto &choice : possible_choice) {
        std::vector<int> seed_clone = seed;
        seed_clone.push_back(choice);
        temp.push_back(seed_clone);
      }
    }
    seeds = temp;
  }
  return seeds;
}

std::vector<int> Sobol::random_seed() {
  std::vector<int> seed(N, 0);
  seed[0] = 1;

  for (int i = 1; i < N; i++) {
    int random = 0;
    while ((random & 1) == 0) {
      random = rand() % (1 << (i + 1));
    }
    seed[i] = random;
  }
  return seed;
}
