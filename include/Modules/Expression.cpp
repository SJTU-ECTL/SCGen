#include "Expression.h"
#include <ginac/inifcns.h>

void SC_sim::Expression::print() {
  // std::cout << "symbols: ";
  // for (const auto &[name, name_symbols] : symbols) {
  //   std::cout << name << "--" << name_symbols.size() << ", ";
  // }
  // std::cout << std::endl;
  std::cout << this->expression << std::endl;
}
SC_sim::Expression SC_sim::Expression::xor_op(Expression a, Expression b) {
  Expression new_ex;
  new_ex.expression = abs(a.expression - b.expression);
  return new_ex;
}

SC_sim::Expression SC_sim::Expression::tanh(Expression a) {
  Expression new_ex;
  new_ex.expression = GiNaC::tanh(a.expression);
  return new_ex;
}

SC_sim::Expression SC_sim::Expression::add(Expression a, Expression b) {
  Expression new_ex;
  new_ex.expression = a.expression + b.expression;
  return new_ex;
}
SC_sim::Expression SC_sim::Expression::constant(double constant) {
  Expression new_ex;
  new_ex.expression = constant;
  return new_ex;
}
SC_sim::Expression SC_sim::Expression::scaled_add(Expression a, Expression b,
                                                  Expression scale) {
  Expression new_ex;
  new_ex.expression =
      (1 - scale.expression) * a.expression + scale.expression * b.expression;
  return new_ex;
}
SC_sim::Expression SC_sim::Expression::scaled_add(Expression a, Expression b,
                                                  double scale) {
  Expression new_ex;
  new_ex.expression = scale * a.expression + (1 - scale) * b.expression;
  return new_ex;
}
SC_sim::Expression SC_sim::Expression::invert(Expression a) {
  Expression new_ex;
  new_ex.expression = 1 - a.expression;
  return new_ex;
}
SC_sim::Expression SC_sim::Expression::or_op(Expression a, Expression b) {
  Expression new_ex;
  new_ex.expression = a.expression + b.expression - a.expression * b.expression;
  return new_ex;
}
SC_sim::Expression SC_sim::Expression::mult(Expression a, Expression b) {
  Expression new_ex;
  new_ex.expression = a.expression * b.expression;
  return new_ex;
}
void SC_sim::Expression::finalize() {
  // Remove all polynomial terms
  expression = expression.expand();
  // std::cout << "Expend: " << expression << std::endl;
  expression =
      expression.subs(pow(GiNaC::wild(0), GiNaC::wild(1)) == GiNaC::wild(0));
  for (const auto &[name, name_symbols] : symbols) {
    for (int i = name_symbols.size() - 1; i >= 1; i--) {
      // std::cout << "replace " << name_symbols[i] << " to " <<
      // name_symbols[0]
      //           << std::endl;
      expression = expression.subs(name_symbols[i] == name_symbols[0]);
      // std::cout << expression << std::endl;
    }
    expression = expression.subs(name_symbols[0] == GiNaC::symbol(name));
  }
  // std::cout << "Finalize: " << expression << std::endl;
}
void SC_sim::Expression::delay(int num) {
  // std::cout << "Delay function()" << std::endl;
  // std::cout << "Before: " << expression << std::endl;
  for (auto &[name, name_symbols] : symbols) {
    int size = name_symbols.size() + num;
    GiNaC::symbol new_symbol;
    while (name_symbols.size() != size) {
      new_symbol = GiNaC::symbol(name + std::to_string(name_symbols.size()));
      name_symbols.push_back(new_symbol);
    }

    for (int i = name_symbols.size() - 1 - num; i >= 0; i--) {
      expression = expression.subs(name_symbols[i] == name_symbols[i + num]);
    }
  }
  // std::cout << "After: " << expression << std::endl;
}
void SC_sim::Expression::add_symbol(std::string name) {
  if (!symbols.contains(name)) {
    std::vector<GiNaC::symbol> name_symbols;
    name_symbols.push_back(GiNaC::symbol(name));
    symbols[name] = name_symbols;
  }
  expression = symbols[name][0];
}

std::unordered_map<std::string, std::vector<GiNaC::symbol>>
    SC_sim::Expression::symbols =
        std::unordered_map<std::string, std::vector<GiNaC::symbol>>();