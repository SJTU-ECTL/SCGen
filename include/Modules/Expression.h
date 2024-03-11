#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <ginac/ginac.h>
#include <ginac/symbol.h>
namespace SC_sim {

class Expression {
private:
  static std::unordered_map<std::string, std::vector<GiNaC::symbol>> symbols;
  GiNaC::ex expression;

public:
  Expression(){};

  void add_symbol(std::string name);

  void print();

  void delay(int num);

  void finalize();

  static Expression mult(Expression a, Expression b);
  static Expression or_op(Expression a, Expression b);
  static Expression invert(Expression a);
  static Expression scaled_add(Expression a, Expression b, double scale);
  static Expression scaled_add(Expression a, Expression b, Expression scale);
  static Expression constant(double constant);
  static Expression add(Expression a, Expression b);
  static Expression xor_op(Expression a, Expression b);
  static Expression tanh(Expression a);

  // FIXME: Add more functions.
};
}   // namespace SC_sim

#endif