#ifndef STOCHASTIC_COMPUTING_GATES_H
#define STOCHASTIC_COMPUTING_GATES_H
#include "Headers.h"

bitstream_list AND(const std::vector<bitstream_list> &inputs);
int AND(const std::vector<int> &inputs);

bitstream_list NAND(const std::vector<bitstream_list> &inputs);
int NAND(const std::vector<int> &inputs);

#endif // STOCHASTIC_COMPUTING_GATES_H
