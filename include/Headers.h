#ifndef STOCHASTIC_COMPUTING_HEADERS_H
#define STOCHASTIC_COMPUTING_HEADERS_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <unordered_set>
#include <vector>

typedef std::vector<int> bitstream_list;
typedef std::vector<int> WCI_set;
typedef std::pair<int,int> CI_pair;
typedef std::vector<int> Clique;
typedef std::vector<Clique> CliqueList;

#define RANDOM_TIMES 100000
#define USE_MSE 1

// Used in Nodes/* to help debugging
#define LOG 0

// #define DEBUG 1

#endif // STOCHASTIC_COMPUTING_HEADERS_H
