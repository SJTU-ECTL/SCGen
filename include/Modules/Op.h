#ifndef OP_H
#define OP_H

#include <vector>

namespace SC_sim {
class Var;
class Op {
public:
  // TODO: Seperate arithmetic operations and gate operations into two classes.
  // TODO: Change the function name to Camel-case
  static Var AND(const std::vector<Var> &vec);
  static Var BinaryAbsSub(const Var &first, const Var &second);
  static Var DFF(const Var &first, int DFF_n);
  static Var Xor(const std::vector<Var> &vec);
  static Var XNOR(const std::vector<Var> &vec);
  static Var ScalingAdd(const Var &first, const Var &second,
                        const Var &scaling_factor);
  static Var SaturAdd(const std::vector<Var> &vec);
  static Var TANH(const Var &X, int scaling_factor = 1);
  static Var OR(const std::vector<Var> &vec);

  static Var Mux(const std::vector<Var> &inputs, const Var &select);
  static Var FSMXK(const Var &X, const Var &K);

  // TODO: Merge it to OR(vector)
  static Var OR(const Var &first, const Var &second);

  static Var LUT(const Var &first);
};
}   // namespace SC_sim
#endif