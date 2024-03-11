#include "Op.h"
#include "Modules/Expression.h"
#include "Var.h"

SC_sim::Var SC_sim::Op::XNOR(const std::vector<Var> &vec) {
  SC_sim::Var ret(vec);
  auto *xnor_node = new XnorNode(vec.size(), 0, vec[0].conf);
  int i = 0;
  for (const auto &input : vec) {
    Node *previous = input.graph.tail;
    previous->connect(previous->outputs.size(), xnor_node, i++);
  }

  ret.graph.tail = xnor_node;

  ret.graph.expression = Expression::mult(
      vec[0].graph.expression,
      vec[1].graph.expression);   // TODO not sure if it is correct

  for (int j = 2; j < vec.size(); j++) {
    ret.graph.expression =
        Expression::mult(ret.graph.expression, vec[j].graph.expression);
  }
  return ret;
}

SC_sim::Var SC_sim::Op::AND(const std::vector<Var> &vec) {
  SC_sim::Var ret(vec);
  auto *and_node = new AndNode(vec.size(), 0, vec[0].conf);
  int i = 0;
  for (const auto &input : vec) {
    Node *previous = input.graph.tail;
    previous->connect(previous->outputs.size(), and_node, i++);
  }

  ret.graph.tail = and_node;

  ret.graph.expression =
      Expression::mult(vec[0].graph.expression, vec[1].graph.expression);

  for (int j = 2; j < vec.size(); j++) {
    ret.graph.expression =
        Expression::mult(ret.graph.expression, vec[j].graph.expression);
  }
  return ret;
}

SC_sim::Var SC_sim::Op::OR(const std::vector<Var> &vec) {
  SC_sim::Var ret(vec);
  auto *or_node = new OrNode(vec.size(), 0, vec[0].conf);
  int i = 0;
  for (const auto &input : vec) {
    Node *previous = input.graph.tail;
    previous->connect(previous->outputs.size(), or_node, i++);
  }

  ret.graph.tail = or_node;

  ret.graph.expression =
      Expression::or_op(vec[0].graph.expression, vec[1].graph.expression);

  for (int j = 2; j < vec.size(); j++) {
    ret.graph.expression =
        Expression::or_op(ret.graph.expression, vec[j].graph.expression);
  }
  return ret;
}

SC_sim::Var SC_sim::Op::DFF(const SC_sim::Var &first, int DFF_n) {
  if (DFF_n == 0) {
    return first;
  }
  SC_sim::Var ret(first);

  Node *previous_a = first.graph.tail;

  auto *DFF_node = new DFFNode(1, 0, first.conf);
  DFF_node->init({std::to_string(DFF_n)});

  previous_a->connect(previous_a->outputs.size(), DFF_node, 0);

  ret.graph.tail = DFF_node;
  ret.graph.expression = first.graph.expression;
  ret.graph.expression.delay(DFF_n);

  return ret;
}

SC_sim::Var SC_sim::Op::Xor(const std::vector<Var> &vec) {
  SC_sim::Var ret(vec);
  auto *xor_node = new XorNode(vec.size(), 0, vec[0].conf);
  int i = 0;
  for (const auto &input : vec) {
    Node *previous = input.graph.tail;
    previous->connect(previous->outputs.size(), xor_node, i++);
  }

  ret.graph.tail = xor_node;

  ret.graph.expression =
      Expression::or_op(vec[0].graph.expression, vec[1].graph.expression);

  for (int j = 2; j < vec.size(); j++) {
    ret.graph.expression =
        Expression::or_op(ret.graph.expression, vec[j].graph.expression);
  }   // TODO: No support for Xor
  return ret;
}
SC_sim::Var SC_sim::Op::BinaryAbsSub(const SC_sim::Var &first,
                                     const SC_sim::Var &second) {
  SC_sim::Var ret(first, second);

  Node *previous_a = first.graph.tail;
  Node *previous_b = second.graph.tail;

  auto *binaryAbsSubNode = new BinaryAbsSubNode(2, 0, first.conf);

  previous_a->connect(previous_a->output_num, binaryAbsSubNode, 0);
  previous_b->connect(previous_b->output_num, binaryAbsSubNode, 1);

  ret.graph.tail = binaryAbsSubNode;

  ret.graph.expression =
      Expression::mult(first.graph.expression, second.graph.expression);
  // TODO: No support for ABSolute Subtraction

  return ret;
}

SC_sim::Var SC_sim::Op::ScalingAdd(const Var &first, const Var &second,
                                   const Var &scaling_factor) {

  SC_sim::Var ret({first, second, scaling_factor});

  Node *previous_a = first.graph.tail;
  Node *previous_b = second.graph.tail;
  Node *previous_c = scaling_factor.graph.tail;

  Node *add_node;
  if (first.type == SC)
    add_node = new AddNode(3, 0, first.conf);
  else
    add_node = new BinaryAddNode(3, 0, first.conf);
  previous_a->connect(previous_a->output_num, add_node, 0);
  previous_b->connect(previous_b->output_num, add_node, 1);
  previous_c->connect(previous_c->output_num, add_node, 2);

  ret.graph.tail = add_node;

  ret.graph.expression =
      Expression::scaled_add(first.graph.expression, second.graph.expression,
                             scaling_factor.graph.expression);

  return ret;
}

SC_sim::Var SC_sim::Op::SaturAdd(const std::vector<SC_sim::Var> &vec) {

  SC_sim::Var ret(vec);
  if (vec[0].type == BC) {
    auto *and_node = new BinarySaturAddNode(vec.size(), 0, vec[0].conf);
    int i = 0;
    for (const auto &input : vec) {
      Node *previous = input.graph.tail;
      previous->connect(previous->outputs.size(), and_node, i++);
    }

    ret.graph.tail = and_node;
  } else {
    auto *and_node = new OrNode(vec.size(), 0, vec[0].conf);
    int i = 0;
    for (const auto &input : vec) {
      Node *previous = input.graph.tail;
      previous->connect(previous->outputs.size(), and_node, i++);
    }

    ret.graph.tail = and_node;
  }
  ret.graph.expression =
      Expression::add(vec[0].graph.expression, vec[1].graph.expression);
  for (int j = 2; j < vec.size(); j++) {
    ret.graph.expression =
        Expression::add(ret.graph.expression, vec[j].graph.expression);
  }
  return ret;
}

SC_sim::Var SC_sim::Op::Mux(const std::vector<SC_sim::Var> &inputs,
                            const SC_sim::Var &select) {
  std::vector<Var> inputs_full = inputs;
  inputs_full.emplace_back(select);
  SC_sim::Var ret(inputs_full);
  auto *add_node = new AddNode(inputs_full.size(), 0, inputs_full[0].conf);
  int i = 0;
  for (const auto &input : inputs_full) {
    Node *previous = input.graph.tail;
    previous->connect(previous->outputs.size(), add_node, i++);
  }
  ret.graph.tail = add_node;
  // TODO: expression
  ret.graph.expression = select.graph.expression;
  return ret;
}

SC_sim::Var SC_sim::Op::FSMXK(const SC_sim::Var &X, const SC_sim::Var &K) {
  assert(X.type == SC_sim::SC && K.type == SC_sim::SC);
  SC_sim::Var ret(X, K);
  auto *fsmxk_node = new FSMXKNode(2, 0, X.conf);
  Node *previous_X = X.graph.tail;
  Node *previous_K = K.graph.tail;
  previous_X->connect(previous_X->outputs.size(), fsmxk_node, 0);
  previous_K->connect(previous_K->outputs.size(), fsmxk_node, 1);
  ret.graph.tail = fsmxk_node;
  ret.graph.expression = X.graph.expression;
  // TODO: expression
  return ret;
}

SC_sim::Var SC_sim::Op::TANH(const SC_sim::Var &X, int scaling_factor) {
  // tanh (scaling_factor * X) based on Brown paper
  SC_sim::Var ret(X);

  if (X.type == SC) {
    auto *fsmx_node = new SCTanhNode(1, 0, X.conf, scaling_factor * 2);
    Node *previous_X = X.graph.tail;
    previous_X->connect(previous_X->outputs.size(), fsmx_node, 0);
    ret.graph.tail = fsmx_node;
    ret.type = SC;
  } else {
    auto *fsmx_node = new BinaryTanhNode(1, 0, X.conf, scaling_factor);
    Node *previous_X = X.graph.tail;
    previous_X->connect(previous_X->outputs.size(), fsmx_node, 0);
    ret.graph.tail = fsmx_node;
    ret.type = BC;
  }
  ret.graph.expression = Expression::tanh(Expression::mult(
      X.graph.expression, Expression::constant(scaling_factor)));
  return ret;
}

SC_sim::Var SC_sim::Op::LUT(const SC_sim::Var &first) {
  SC_sim::Var ret(first);
  assert(first.graph.inputs.size() == 2);
  // assert(first.graph.outputs.size() == 1);

  SubcircuitNode *subcircuit = new SubcircuitNode(2, 0, first.conf);
  std::vector<Node *> input_nodes;
  for (const auto &input : first.graph.inputs) {
    for (const auto &out : input->outputs) {
      input_nodes.push_back(out);
    }
  }

  std::vector<Node *> output_nodes;
  output_nodes.push_back(first.graph.tail);
  // for (int i = 0; i < first.graph.outputs.size(); i++)
  //   output_nodes[i] = first.graph.outputs[i];
  subcircuit->add_inside_circuits(input_nodes, output_nodes);

  // Hack
  for (int i = 0; i < first.graph.inputs.size(); i++) {
    first.graph.inputs[i]->connect(0, subcircuit, i);
  }

  // subcircuit->connect(0, first.graph.tail, 0);

  ret.graph.inputs = first.graph.inputs;
  ret.graph.outputs = first.graph.outputs;
  ret.graph.tail = subcircuit;

  return ret;
}