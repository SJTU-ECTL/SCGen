#include "Var.h"
#include "Nodes/BinaryNode.h"
#include "Nodes/Node.h"
#include "Nodes/SNGNode.h"
#include "Nodes/SourceNode.h"
#include "opencv2/core/base.hpp"
#include <algorithm>
#include <unordered_map>
#include <utility>

SC_sim::Var::Var(const std::vector<Var> &seconds) {
  type = seconds[0].type;
  conf = seconds[0].conf;

  for (auto &second : seconds) {
    for (const auto &input : second.graph.inputs) {
      if (std::find(this->graph.inputs.begin(), this->graph.inputs.end(),
                    input) != this->graph.inputs.end()) {
        continue;
      }
      this->graph.inputs.push_back(input);
    }
  }
}

SC_sim::Var::Var(const Var &a, const Var &b) {
  type = a.type;
  conf = a.conf;

  // size_t i = 0;
  for (const auto &input : a.graph.inputs) {
    this->graph.inputs.push_back(input);
    // TODO
    // this->graph.symbols.push_back(a.graph.symbols.at(i++));
  }
  // i = 0;
  for (const auto &input : b.graph.inputs) {
    if (std::find(this->graph.inputs.begin(), this->graph.inputs.end(),
                  input) != this->graph.inputs.end()) {
      continue;
    }
    this->graph.inputs.push_back(input);
    // if (i < b.graph.symbols.size())
    //   this->graph.symbols.push_back(b.graph.symbols.at(i));
    // i++;
  }
}

int SC_sim::Var::get_input_index(int id) {
  int index = -1;
  for (int i = 0; i < this->graph.inputs.size(); i++) {
    if (this->graph.inputs[i]->id == id) {
      index = i;
      return index;
    }
  }
  return -1;
}

void SC_sim::Var::reorderInput(std::vector<Var> input_seq) {
  std::vector<SourceNode *> new_inputs = {};
  for (auto &i : input_seq) {
    assert(i.graph.inputs.size() == 1);
    int index = get_input_index(i.graph.inputs[0]->id);
    if (index == -1)
      std::cout << "Error occured when reorder inputs." << std::endl;
    new_inputs.emplace_back(this->graph.inputs[index]);
  }
  if (new_inputs.size() != this->graph.inputs.size()) {
    for (auto &i : this->graph.inputs) {
      if (std::find(new_inputs.begin(), new_inputs.end(), i) !=
          new_inputs.end()) {
        // find
        continue;
      }
      new_inputs.emplace_back(i);
    }
  }
  this->graph.inputs = new_inputs;
}

void SC_sim::Var::customUserInput(const std::string &input_file, int height,
                                  int width, std::vector<Var> input_seq) {
  // reformulate input sequence
  // Require input_seq contains only SourceNode
  // That is to say; Only one input
  customUserInput(input_file, height * width, input_seq);
}

void SC_sim::Var::customUserInput(const std::string &input_file, int size,
                                  std::vector<Var> input_seq) {
  // reformulate input sequence
  // Require input_seq contains only SourceNode
  // That is to say; Only one input
  reorderInput(input_seq);
  std::ifstream infile(input_file);
  std::string line;
  std::vector<std::vector<int>> user_inputs(size);
  for (int i = 0; i < size; i++) {
    user_inputs[i] = {};
  }
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    for (int i = 0; i < size; i++) {
      int tmp;
      iss >> tmp;
      user_inputs[i].emplace_back(tmp);
    }
  }

  for (int j = 0; j < size; j++) {
    if (auto *temp = dynamic_cast<UserDefSourceNode *>(this->graph.inputs[j])) {
      temp->setInput(user_inputs[j]);
    }
  }
}

SC_sim::Var SC_sim::Var::BC_add(const Var &second) {
  Var ret(*this, second);

  Node *previous_a = this->graph.tail;
  Node *previous_b = second.graph.tail;

  auto *half = new ConstantSourceNode(0, 0, conf);

  auto *binary_add = new BinaryAddNode(3, 0, conf);
  half->init({"0.5"});

  previous_a->connect(previous_a->output_num, binary_add, 0);
  previous_b->connect(previous_b->output_num, binary_add, 1);
  half->connect(half->output_num, binary_add, 2);

  ret.graph.tail = binary_add;
  ret.graph.inputs.push_back(half);

  ret.graph.expression = Expression::scaled_add(this->graph.expression,
                                                second.graph.expression, 0.5);

  return ret;
}

SC_sim::Var SC_sim::Var::BC_mult(const Var &second) {
  Var ret(*this, second);

  Node *previous_a = this->graph.tail;
  Node *previous_b = second.graph.tail;

  BinaryMulNode *binary_mul = new BinaryMulNode(2, 0, conf);
  previous_a->connect(previous_a->output_num, binary_mul, 0);
  previous_b->connect(previous_b->output_num, binary_mul, 1);

  ret.graph.tail = binary_mul;

  ret.graph.expression =
      Expression::mult(this->graph.expression, second.graph.expression);

  return ret;
}

SC_sim::Var SC_sim::Var::SC_add(const Var &second) {
  Var ret(*this, second);

  Node *previous_a = this->graph.tail;
  Node *previous_b = second.graph.tail;

  auto *add_node = new AddNode(3, 0, conf);

  previous_a->connect(previous_a->output_num, add_node, 0);
  previous_b->connect(previous_b->output_num, add_node, 1);

  Config conf_const = conf;
  conf_const.scType = unipolar;
  // the select signal of Mux Based SC adder is always unipolar

  auto *half = new ConstantSourceNode(0, 1, conf_const);
  half->init({"0.5"});

  SNGNode *half_SNG;

  if (dynamic_cast<SobolNode *>(this->graph.inputs[0]->outputs[0]) != NULL)
    half_SNG = new SobolNode(1, 1, conf_const);
  else
    half_SNG = new LFSRNode(1, 1, conf_const);

  half_SNG->init({});

  half->connect(0, half_SNG, 0);
  half_SNG->connect(0, add_node, 2);

  ret.graph.inputs.push_back(half);

  ret.graph.tail = add_node;

  ret.graph.expression = Expression::scaled_add(this->graph.expression,
                                                second.graph.expression, 0.5);

  return ret;
}

SC_sim::Var SC_sim::Var::BC_to_SC(const Var &second, const Var *copyFrom) {
  // if copyFrom is not NULL,
  // user has to make sure that the tail of copyFrom is LFSR/SNG
  Var ret = second;
  ret.type = SC;

  Node *previous = second.graph.tail;

  // add SNG
  LFSRNode *SNG;

  if (copyFrom == NULL)
    SNG = new LFSRNode(1, 0, ret.conf);
  else {
    SNGNode *tmp = dynamic_cast<SNGNode *>(copyFrom->graph.tail);
    SNG = new LFSRNode(1, 0, ret.conf, {tmp});
  }
  SNG->init({});

  previous->connect(previous->output_num, SNG, 0);

  ret.graph.tail = SNG;

  return ret;
}

SC_sim::Var SC_sim::Var::SC_to_BC(const Var &second) {
  Var ret = second;
  ret.type = BC;

  Node *previous = second.graph.tail;

  // add CounterNode
  auto *counter = new CounterNode(1, 0, ret.conf);
  previous->connect(previous->output_num, counter, 0);

  ret.graph.tail = counter;

  return ret;
}

SC_sim::Var SC_sim::Var::constantVar(float num, VAR_TYPE type,
                                     const Config &_conf,
                                     const SNG_TYPE &_sng_type, Var *copyFrom,
                                     std::vector<std::string> extra) {
  Var ret;
  ret.type = type;
  ret.conf = _conf;

  ConstantSourceNode *in_node;

  SNGNode *SNG_a;
  if (type == BC) {
    in_node = new ConstantSourceNode(0, 0, _conf);
    in_node->init({std::to_string(num)});
  } else {
    in_node = new ConstantSourceNode(0, 1, _conf);
    in_node->init({std::to_string(num)});

    // TODO: add similar support for self-copyFrom for others
    if (_sng_type == SOBOL_SNG) {
      SNG_a = new SobolNode(1, 0, ret.conf);
    } else {
      if (copyFrom == nullptr)
        SNG_a = new LFSRNode(1, 0, ret.conf);
      else {
        auto *tmp = dynamic_cast<SNGNode *>(copyFrom->graph.tail);
        if (tmp == nullptr) {
          SNG_a = new LFSRNode(1, 0, ret.conf);
        } else {
          SNG_a = new LFSRNode(1, 0, ret.conf, {tmp});
        }
      }
    }
    SNG_a->init(extra);
    // SNG_a->randomize();

    in_node->connect(0, SNG_a, 0);
  }
  ret.graph.inputs.push_back(in_node);
  // ret.graph.outputs.push_back(out_node);
  ret.graph.expression = Expression::constant(num);
  if (type == SC)
    ret.graph.tail = SNG_a;
  else
    ret.graph.tail = in_node;
  return ret;
}

SC_sim::Var SC_sim::Var::new_SC(const Config &_conf, const SNG_TYPE &type,
                                const Var *copyFrom,
                                const GiNaC::symbol &_symbol) {
  Var ret;
  ret.type = SC;
  ret.conf = _conf;

  SourceNode *in_node;
  switch (_conf.simType) {
  case full_sim:
    in_node = new SourceNode(0, 1, ret.conf);
    break;
  case rand_sim:
    in_node = new RandomSourceNode(0, 1, ret.conf);
    break;
  case user_def_sim:
    in_node = new UserDefSourceNode(0, 1, ret.conf);
    break;
  default:
    in_node = new SourceNode(0, 1, ret.conf);
    break;
  }

  SNGNode *SNG_a;
  // TODO: add similar support for self-copyFrom for others
  if (copyFrom == nullptr) {
    switch (type) {
    case LFSR_SNG: {
      SNG_a = new LFSRNode(1, 0, ret.conf);
      break;
    }
    case SOBOL_SNG: {
      SNG_a = new SobolNode(1, 0, ret.conf);
      break;
    }
    default: {
      SNG_a = new LFSRNode(1, 0, ret.conf);
      break;
    }
    }
  } else {
    // TODO: sharing of Sobol SNG
    auto *tmp = dynamic_cast<SNGNode *>(copyFrom->graph.tail);
    if (tmp == nullptr) {
      SNG_a = new LFSRNode(1, 0, ret.conf);
    } else {
      SNG_a = new LFSRNode(1, 0, ret.conf, {tmp});
    }
  }
  SNG_a->init({});
  // SNG_a->randomize();

  in_node->connect(0, SNG_a, 0);

  ret.graph.inputs.push_back(in_node);
  // ret.graph.symbols.push_back(_symbol);
  ret.graph.expression.add_symbol(_symbol.get_name());
  ret.graph.tail = SNG_a;

  return ret;
}

SC_sim::Var SC_sim::Var::new_BC(const Config &_conf) {
  Var ret;
  ret.type = BC;
  ret.conf = _conf;

  SourceNode *in_node;
  switch (_conf.simType) {
  case full_sim:
    in_node = new SourceNode(0, 0, ret.conf);
    break;
  case rand_sim:
    in_node = new RandomSourceNode(0, 0, ret.conf);
    break;
  case user_def_sim:
    in_node = new UserDefSourceNode(0, 0, ret.conf);
    break;
  default:
    break;
  }

  ret.graph.inputs.push_back(in_node);
  ret.graph.tail = in_node;
  //      ret.graph.symbols.push_back(_symbol);
  //      ret.graph.expression = _symbol;
  return ret;
}

SC_sim::Var::Var(const VAR_TYPE &_type, const Config &_conf,
                 const SNG_TYPE &_sng_type, const Var *copyFrom,
                 const GiNaC::symbol &_symbol) {
  // this->type = UNINITIALIZE;
  if (_type == SC) {
    *this = new_SC(_conf, _sng_type, copyFrom, _symbol);
  } else if (_type == BC) {
    *this = new_BC(_conf);
  }
}

SC_sim::Var::Var(const std::string &type_str, const Config &_conf) {
  if (type_str.compare("SC") == 0) {
    Var(VAR_TYPE::SC, _conf);
  } else if (type_str.compare("BC") == 0) {
    Var(VAR_TYPE::BC, _conf);
  }
}

SC_sim::Var::Var(enum VAR_TYPE _type, Config _conf,
                 const std::string &file_path) {
  type = _type;
  conf = _conf;
  std::unordered_map<std::string, Node *> node_map;
  std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                            std::vector<std::string>>>
      name_map;
  parse_file(file_path, conf, node_map, name_map);

  int calculate_num = 0;
  for (const auto &[name, node] : node_map) {
    // remove CalculateNode
    if (auto *temp = dynamic_cast<CalculateNode *>(node)) {
      bool flag = false;

      for (const auto &in : temp->inputs) {
        in->outputs[in->find_out_node(temp)] = in->outputs[in->output_num - 1];
        in->outputs.pop_back();
        in->output_num--;
        // in->outputs[in->find_out_node(temp)] = nullptr;
        if (dynamic_cast<SNGNode *>(in)) {
          flag = true;
        }
      }
      if (flag)
        calculate_num++;
    }
    // add inputs
    if (auto *temp = dynamic_cast<SourceNode *>(node)) {
      this->graph.inputs.push_back(temp);
    }

    // add output
    if (auto *temp = dynamic_cast<DestNode *>(node)) {
      this->graph.outputs.push_back(temp);
    }
  }
  // reduce the number of output of SNGNode
  // for (const auto &[name, node] : node_map) {
  //   if (SNGNode *temp = dynamic_cast<SNGNode *>(node)) {
  //     for (int i = 0; i < calculate_num; i++) {
  //       temp->outputs.pop_back();
  //     }
  //     temp->output_num -= calculate_num;
  //   }
  // }
}

SC_sim::Var::Var(const std::string &type_str, Config _conf,
                 std::string file_path) {
  type = type_str.compare("SC") == 0 ? SC : BC;
  conf = _conf;
}

void SC_sim::Var::operator=(const Var &second) {
  this->conf = second.conf;
  // Used when LHS is uninitialized. Avoid one DAG copy.
  if (this->type == UNINITIALIZE) {
    this->graph.borrow(second.graph);
  } else {
    this->graph = second.graph;
  }
  this->type = second.type;
}

SC_sim::Var SC_sim::Var::copy() {
  Var new_var;
  new_var.conf = this->conf;
  new_var.type = this->type;
  new_var.graph = this->graph;
  return new_var;
}

SC_sim::Var SC_sim::Var::operator*(const Var &second) {
  Var second_new = second;
  if (this->type == SC) {
    if (second.type == BC) {
      second_new = BC_to_SC(second);
    }
    if (this->conf.scType == unipolar)
      return SC_sim::Op::AND({*this, second_new});
    else   // bipolar
      return SC_sim::Op::XNOR({*this, second_new});
  } else {   // type == BC
    if (second.type == SC) {
      second_new = SC_to_BC(second);
    }
    return BC_mult(second_new);
  }
}

SC_sim::Var SC_sim::Var::to_BC() {
  assert(this->type == SC);
  return SC_to_BC(*this);
}

SC_sim::Var SC_sim::Var::to_SC() {
  assert(this->type == BC);
  return BC_to_SC(*this);
}

SC_sim::Var SC_sim::Var::apc(std::vector<Var> seconds) {
  Var ret(seconds);
  ret.type = BC;
  auto *apcNode = new APCNode(seconds.size(), 0, seconds.front().conf);
  int i = 0;
  for (auto &second : seconds) {
    Node *previous = second.graph.tail;
    previous->connect(previous->output_num, apcNode, i++);
  }
  ret.graph.tail = apcNode;

  for (const auto &second : seconds) {
    ret.graph.expression =
        Expression::add(ret.graph.expression, second.graph.expression);
  }
  return ret;
}

SC_sim::Var SC_sim::Var::BinarySum(std::vector<Var> seconds) {
  Var ret(seconds);
  ret.type = BC;
  auto *binarySumNode =
      new BinarySumNode(seconds.size(), 0, seconds.front().conf);
  int i = 0;
  for (auto &second : seconds) {
    Node *previous = second.graph.tail;
    previous->connect(previous->output_num, binarySumNode, i++);
  }
  ret.graph.tail = binarySumNode;

  for (const auto &second : seconds) {
    ret.graph.expression =
        Expression::add(ret.graph.expression, second.graph.expression);
  }
  return ret;
}

SC_sim::Var SC_sim::Var::operator+(const Var &second) {
  Var second_new = second;
  if (this->type == SC) {
    if (second.type == BC) {
      second_new = BC_to_SC(second);
    }
    return SC_add(second_new);
  } else {   // type == BC
    if (second.type == SC) {
      second_new = SC_to_BC(second);
    }
    return BC_add(second_new);
  }
}

SC_sim::Var SC_sim::Var::abs_sub(const Var &second) {
  auto transformedFirst = *this;
  auto transformedSecond = second;
  if (this->type == SC) {
    if (second.type == BC) {
      transformedSecond = BC_to_SC(second, &transformedFirst);
    }
    return Op::Xor({transformedFirst, transformedSecond});
  } else {
    // BC
    if (second.type == SC) {
      transformedSecond = SC_to_BC(second);
    }
    return Op::BinaryAbsSub(transformedFirst, transformedSecond);
  }
}

SC_sim::Var SC_sim::Var::Randomizer(const Var &second, const Var *copyFrom) {
  // currently only implements this function for SC
  assert(second.type == BC);
  SC_sim::Var ret({second});

  Node *previous = second.graph.tail;

  SNGNode *SNG;
  if (copyFrom == NULL) {
    if (dynamic_cast<SobolNode *>(second.graph.inputs[0]->outputs[0]) != NULL)
      SNG = new SobolNode(1, 0, ret.conf);
    else
      SNG = new LFSRNode(1, 0, ret.conf);
  } else {
    SNGNode *tmp = dynamic_cast<SNGNode *>(copyFrom->graph.tail);
    if (dynamic_cast<SobolNode *>(second.graph.inputs[0]->outputs[0]) != NULL)
      SNG = new SobolNode(1, 0, ret.conf, {tmp});
    else
      SNG = new LFSRNode(1, 0, ret.conf, {tmp});
  }
  SNG->init({});

  previous->connect(previous->output_num, SNG, 0);

  ret.type = SC;

  ret.graph.tail = SNG;

  ret.graph.expression = second.graph.expression;

  return ret;
}

SC_sim::Var SC_sim::Var::Counter(const Var &second) {
  // currently only implements this function for SC
  assert(second.type == SC);
  SC_sim::Var ret({second});

  Node *previous = second.graph.tail;

  auto *counter = new CounterNode(1, 0, ret.conf);

  previous->connect(previous->output_num, counter, 0);

  ret.type = BC;

  ret.graph.tail = counter;

  ret.graph.expression = second.graph.expression;

  return ret;
}

SC_sim::Var SC_sim::Var::sum(const std::vector<Var> &seconds) {
  std::vector<Var> transformedSeconds = {};
  int num_b = 0;
  int num_s = 0;
  for (const auto &second : seconds) {
    if (second.type == SC)
      num_s++;
    else
      num_b++;
  }
  for (const auto &second : seconds) {
    if (num_s >= num_b)
      if (second.type == BC) {
        transformedSeconds.emplace_back(BC_to_SC(second));
      } else {
        transformedSeconds.emplace_back(second);
      }
    else if (second.type == BC) {
      transformedSeconds.emplace_back(second);
    } else {
      transformedSeconds.emplace_back(SC_to_BC(second));
    }
  }
  if (num_s >= num_b)
    return apc(transformedSeconds);
  return BinarySum(transformedSeconds);
}

SC_sim::Var SC_sim::Var::pc(const std::vector<Var> &seconds) {
  // Var:  Sum   vs    PC
  // BC: BinarySum vs BinarySum
  // SC: APC     vs   BinarySum
  // PC is a special case for CI_example_15
  for (const auto &second : seconds) {
    assert(second.type == SC);
  }
  return BinarySum(seconds);
}

SC_sim::Var operator-(int value, const SC_sim::Var &second) {
  if (value != 1) {
    std::cout << "value not being 1 is not supported." << std::endl;
    exit(-1);
  }

  SC_sim::Var ret = second;

  Node *previous = second.graph.tail;

  // add InverterNode
  auto *inverter = new NNode(1, 0, ret.conf);
  previous->connect(previous->output_num, inverter, 0);

  ret.graph.tail = inverter;

  ret.graph.expression = SC_sim::Expression::invert(ret.graph.expression);

  return ret;
}

void SC_sim::Var::reset() {
  this->graph.inputs[0]->clear_ready();
  for (const auto &node : this->graph.all_nodes) {
    node->reset();
  }
}

void SC_sim::Var::finish() {
  for (const auto &node : this->graph.all_nodes) {
    node->finish();
  }
}

void SC_sim::Var::addCalculate(
    std::string type_str,
    std::function<double(const std::vector<double> &)> _fn) {
  if (this->graph.all_nodes.empty())
    this->graph.generate_all_nodes();

  // Add DestNode
  DestNode *dest_node;
  if (this->type == SC) {
    dest_node = new DestNode(1, 1, conf);
  } else {
    dest_node = new BinaryDestNode(1, 1, conf);
  }
  this->graph.tail->connect(0, dest_node, 0);
  this->graph.outputs.push_back(dest_node);
  int last_index = this->graph.node_id_dict[this->graph.all_nodes.back()];
  this->graph.node_id_dict[dest_node] = ++last_index;
  this->graph.all_nodes.emplace_back(dest_node);

  CalculateNode *calculate;
  if (type_str.compare("MSE") == 0) {
    calculate = new MSENode(1, 1, conf);
  } else {
    calculate = new MAENode(1, 1, conf);
  }
  this->graph.node_id_dict[calculate] = ++last_index;
  calculate->set_fn(_fn);
  this->graph.all_nodes.emplace_back(calculate);

  MinNode *min_calculate = new MinNode(1, 0, conf);
  this->graph.calculates.push_back(min_calculate);
  this->graph.all_nodes.emplace_back(min_calculate);
  this->graph.node_id_dict[min_calculate] = ++last_index;

  int index = 0;
  for (const auto &input : this->graph.inputs) {
    input->connect(input->output_num, calculate, index++);
  }
  this->graph.outputs[0]->connect(0, calculate, index++);

  calculate->connect(0, min_calculate, 0);

  for (const auto &sng_node : this->graph.SNGs) {
    sng_node->connect(sng_node->outputs.size(), min_calculate,
                      min_calculate->inputs.size());
  }
}

void SC_sim::Var::optimize() {
  if (conf.optimize & RUNNING_MINIMAL) {
    for (const auto &input : graph.inputs) {
      input->minNode = graph.calculates[0];
    }
  }
}

void SC_sim::Var::simulate_reset() {
  for (const auto &cal : this->graph.calculates) {
    cal->clear();
  }
}

void SC_sim::Var::search(double target_error) {
  Var circuit = *this;
  // double error = circuit.simulate();
  double error = 0;
  bool end = false;
  int index = 0;
  while (!end) {
    index++;
    std::cout << "=========="
              << " " << index << "th replacement:"
              << " "
              << "==========" << std::endl;
    end = find(circuit, error, target_error);
    // std::cout << end << std::endl;
  }
  std::cout << "error find: " << error << std::endl;
}

bool SC_sim::Var::find(Var &circuit, double &error,
                       const double &target_error) {
  std::vector<Node *> BC_nodes = circuit.all_BC_nodes();
  if (BC_nodes.empty())
    return true;

  std::cout << BC_nodes.size() << std::endl;
  double min_error = 100000;
  Var min_circuit = circuit;
  for (const auto node : BC_nodes) {
    Var new_circuit = circuit.replace(node);

    // std::cout << "After replace: " << new_circuit.all_BC_nodes().size()
    //           << std::endl;

    // TODO: implement this later, since simulate() return value has changed.
    // double new_error = new_circuit.simulate();
    double new_error = -1;
    if (new_error < min_error) {
      min_error = new_error;
      min_circuit = new_circuit;
    }
    std::cout << "new_error: " << new_error << std::endl;
  }

  if (min_error > target_error) {
    std::cout << "Failed to meet constraint!" << std::endl;
    return true;
  }

  circuit = min_circuit;
  error = min_error;
  std::cout << "Meet meet constraint with error " << min_error << "."
            << std::endl;
  return false;
}

SC_sim::Var SC_sim::Var::replace(Node *node) {
  assert(dynamic_cast<BinaryNode *>(node));

  int node_index = 0;
  for (Node *t : this->graph.all_nodes) {
    if (t == node)
      break;
    node_index++;
  }

  // std::cout << "Node index: " << node_index << std::endl;

  Var new_var = this->copy();

  node = new_var.graph.all_nodes[node_index];

  Node *SC_node = SC_equivalence(node);
  // Find previous nodes, if BC or SourceNode, add BC_to_SC convertor
  for (const auto &previous : node->inputs) {
    int out_index = Node::find_out_node(previous, node);
    int in_index = Node::find_in_node(previous, node);
    if (dynamic_cast<BinaryNode *>(previous) ||
        dynamic_cast<SourceNode *>(previous)) {
      LFSRNode *SNG = new LFSRNode(1, 0, node->conf);
      SNG->init({});

      previous->connect(out_index, SNG, 0);

      SNG->connect(0, SC_node, in_index);
    } else {
      previous->connect(out_index, SC_node, in_index);
    }
  }

  if (dynamic_cast<AddNode *>(SC_node)) {
    std::cout << "special treatment for AddNode." << std::endl;

    ConstantSourceNode *constant = new ConstantSourceNode(0, 1, node->conf);
    constant->set_value(0.5);
    new_var.graph.inputs.push_back(constant);

    int len = new_var.graph.inputs.size();

    new_var.graph.inputs[len - 2]->set_next_node(constant);

    LFSRNode *SNG = new LFSRNode(1, 0, node->conf);
    SNG->init({});

    constant->connect(0, SNG, 0);

    SNG->connect(0, SC_node, 2);
  }

  // Find after nodes, if BC, add SC_to_BC convertor
  for (const auto &after : node->outputs) {
    int out_index = Node::find_out_node(node, after);
    int in_index = Node::find_in_node(node, after);
    if (dynamic_cast<BinaryNode *>(after) ||
        dynamic_cast<BinaryDestNode *>(after)) {

      CounterNode *counter = new CounterNode(1, 0, node->conf);
      SC_node->connect(out_index, counter, 0);
      counter->connect(0, after, in_index);
    } else {
      SC_node->connect(out_index, after, in_index);
    }
  }

  new_var.graph.generate_all_nodes();
  return new_var;
}

Node *SC_sim::Var::SC_equivalence(Node *node) {
  if (dynamic_cast<BinaryMulNode *>(node)) {
    return new AndNode(node->input_num, node->output_num, node->conf);
  } else if (dynamic_cast<BinaryNormAddNode *>(node)) {
    return new AddNode(node->input_num, node->output_num, node->conf);
  } else if (dynamic_cast<BinaryAddNode *>(node)) {
    return new AddNode(node->input_num, node->output_num, node->conf);
  }
  assert(0 && "not implemented");
  return nullptr;
}

std::vector<Node *> SC_sim::Var::all_BC_nodes() {
  std::vector<Node *> ret;
  for (const auto node : this->graph.all_nodes) {
    if (dynamic_cast<BinaryNode *>(node)) {
      if (dynamic_cast<CounterNode *>(node))
        continue;

      ret.push_back(node);
    }
  }
  return ret;
}

void SC_sim::Var::clear_graph() {
  for (auto &input : this->graph.inputs) {
    input->clear_ready();
  }
}

void SC_sim::Var::expr() {
  // std::cout << "Before finalize: " << std::endl;
  // this->graph.expression.print();

  this->graph.expression.finalize();
  // std::cout << this->graph.expression << std::endl;
  this->graph.expression.print();
}

void SC_sim::Var::config_by_str(const std::string &str) {
  std::stringstream ss(str);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ss, line, '\n')) {
    if (line.size() == 0)
      continue;
    lines.push_back(line);   // Remove space as well
  }

  // Remove first line
  for (int i = 1; i < lines.size(); i++) {
    std::string line = lines[i];
    std::string::size_type pos = line.find(':');
    std::cout << "|" << line << "|" << std::endl;
    line = line.substr(pos + 2);

    auto calculate_node = this->graph.calculates[0];
    dynamic_cast<SNGNode *>(calculate_node->inputs[i])->config_by_str(line);
  }

  // Set RNS sharing
  std::unordered_map<int, SNGNode *> id_SNG_map;
  for (const auto &sng : this->graph.SNGs)
    id_SNG_map[sng->id] = sng;

  for (const auto &sng : this->graph.SNGs) {
    if (!sng->master_id.empty()) {
      sng->master[0] = id_SNG_map[sng->master_id[0]];
      sng->master_id.clear();
    }
  }
}

std::pair<double, std::string> SC_sim::Var::simulate_method(int rand_times) {
  std::pair<double, std::string> ret;

  std::string method = conf.search_method;

  if (this->conf.rand_till_converge) {
    this->conf.rand_times = 100000000;
  } else if (rand_times != -1) {
    this->conf.rand_times = rand_times;
  }

  AgentConfig agent_config;
  agent_config.NO_IMPROVE_BOUND = 500;
  if (method.compare("RANDOM") == 0) {
    ret = RandomAgent::run(this, agent_config);
  } else if (method.compare("MCTS") == 0) {
    ret = MCTSAgent::run(this, agent_config);
  } else if (method.compare("DESCENT") == 0) {
    ret = DescentAgent::run(this, agent_config);
  } else if (method.compare("SA") == 0) {
    ret = SAAgent::run(this, agent_config);
  } else if (method.compare("SA2") == 0) {
    ret = SA2Agent::run(this, agent_config);
  } else if (method.compare("GA") == 0) {
    ret = GAAgent::run(this, agent_config);
  } else if (method.compare("GA2") == 0) {
    ret = GA2Agent::run(this, agent_config);
  } else {
    ret = RandomAgent::run(this, agent_config);
  }

  std::cout << "Min value found: " << ret.first << std::endl;
  // std::cout << "Min conf found: " << ret.second << std::endl;
  std::cout << ret.second << std::endl;
  return ret;
}

std::pair<double, std::string>
SC_sim::Var::simulate_once(bool accurate_result) {
  // For simulate once, we should clear the status of MinNode to get current
  // configs.
  // But we will not have running minimal acceleration
  if (accurate_result)
    simulate_reset();

  reset();
  this->graph.inputs[0]->run();
  finish();

  double ret_value = -1;
  std::string result;
  assert(this->graph.calculates.size() == 1);
  for (const auto &cal : this->graph.calculates) {
    result = cal->get_result_str();
    size_t found = result.find("value");
    if (found != std::string::npos) {
      size_t end = result.find(' ', found + 6);
      ret_value = std::stod(result.substr(found + 6, end - found - 6));
    }
  }
  return std::pair<double, std::string>(ret_value, result);
}

std::pair<double, std::string> SC_sim::Var::simulate() {
  srand(this->conf.seed);

  optimize();

  if (this->graph.all_nodes.empty())
    this->graph.generate_all_nodes();

  for (const auto node : this->graph.all_nodes) {
    node->clear();
  }

  // Place constant source nodes at the beginning
  std::vector<SourceNode *> new_inputs;
  int non_const_sng_index = 0;
  for (size_t i = 0; i < this->graph.inputs.size(); i++) {
    if (dynamic_cast<ConstantSourceNode *>(this->graph.inputs[i]) != NULL) {
      new_inputs.insert(new_inputs.begin(), this->graph.inputs[i]);
      non_const_sng_index++;
    } else {
      new_inputs.push_back(this->graph.inputs[i]);
    }
  }
  this->graph.inputs = new_inputs;

  for (size_t i = 0; i < this->graph.inputs.size() - 1; i++) {
    assert(this->graph.inputs[i]->get_next_node() == nullptr);
    this->graph.inputs[i]->set_next_node(this->graph.inputs[i + 1]);
  }

  assert(this->graph.inputs[this->graph.inputs.size() - 1]->get_next_node() ==
         nullptr);

  switch (conf.simType) {
  case rand_sim:
    this->graph.inputs[non_const_sng_index]->init(
        {std::to_string(conf.rand_sim_times)});
    break;
  case user_def_sim:
    this->graph.inputs[non_const_sng_index]->init({});
  default:
    break;
  }

  std::cout << "------------------------------ simulation start "
               "------------------------------"
            << std::endl;
  std::cout << conf << std::endl;

  auto time_start = std::chrono::high_resolution_clock::now();

  // Start simulation
  std::pair<double, std::string> ret = simulate_method();

  auto time_end = std::chrono::high_resolution_clock::now();

  auto time =
      std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start);

  std::cout << std::endl;
  std::cout << "Time cost: " << time.count() << " seconds." << std::endl;

  std::cout << "------------------------------ simulation end   "
               "------------------------------"
            << std::endl;

  return ret;
}

void SC_sim::Var::genVerilog() {
  std::string file_name =
      "../Verilog/" + conf.graph_name + "/" + conf.graph_name + ".v";

  std::system((std::string("rm -r ../Verilog/") + conf.graph_name).c_str());
  std::system((std::string("mkdir -p ../Verilog/") + conf.graph_name).c_str());

  std::ofstream outfile(file_name);

  std::vector<int> input_index = {};   // index of all non-const input
  for (const auto &input_node : this->graph.inputs) {
    if (dynamic_cast<SourceNode *>(input_node) != nullptr &&
        dynamic_cast<ConstantSourceNode *>(input_node) == nullptr) {
      input_index.emplace_back(input_node->id);
    }
  }

  outfile << "module " << this->conf.graph_name << "(\n";
  outfile << "\t\tclk,\n";
  outfile << "\t\trst,\n";
  for (auto bin : input_index) {
    outfile << "\t\tb_in" << bin << ",\n";
  }

  outfile << "\t\tb_dest\n";
  outfile << ");\n";

  outfile << "\tparameter bit_width = " << conf.bit_width << ";\n"
          << "\n";
  // input wires declaration
  outfile << "\tinput clk;\n";
  outfile << "\tinput rst;\n";
  outfile << "\tinput [bit_width-1:0] ";
  int bin_index = 0;
  for (; bin_index < input_index.size() - 1; bin_index++) {
    outfile << "b_in" << input_index[bin_index] << ",";
  }
  outfile << "b_in" << input_index[bin_index] << ";\n";

  if (dynamic_cast<BinaryDestNode *>(this->graph.outputs[0])) {
    // BinaryDestNode
    outfile << "\toutput [bit_width-1:0] b_dest;\n";
  } else {
    outfile << "\toutput reg [bit_width-1:0] b_dest;\n";
  }
  outfile << "\n";

  // Wire connection
  for (const auto &node : this->graph.all_nodes) {
    if (node->verilog_has_instance()) {
      for (const auto &out : node->outputs) {
        if (out->verilog_has_instance()) {
          if (dynamic_cast<BinaryNode *>(node) != nullptr ||
              dynamic_cast<SourceNode *>(node) != nullptr) {
            // Wires with multiple bits
            outfile << "\twire [bit_width-1:0] "
                    << node->verilog_wire_name(node, out) << ";\n";
          } else {
            // Wires with one bit
            outfile << "\twire " << node->verilog_wire_name(node, out) << ";\n";
          }
        }
      }
    }
  }
  outfile << "\n";

  // instantiate module
  for (const auto &node : this->graph.all_nodes) {
    std::string init = node->verilog_module_init_str();
    if (init.size() != 0) {
      std::cout << "verilog_module_init_str() returns: "
                   "|"
                << init << "|" << std::endl;
      outfile << init << std::endl;
    }
    std::string module = node->verilog_codegen_str(
        exclude_non_verilog_node(this->graph.node_id_dict));
    if (module.size() != 0) {
      std::cout << "verilog_codegen_str() returns: "
                   "|"
                << module << "|" << std::endl;
      outfile << module << std::endl;
    }
    std::string multi_output = node->verilog_codegen_multi_output(
        exclude_non_verilog_node(this->graph.node_id_dict));
    if (multi_output.size() != 0) {
      std::cout << "verilog_codegen_multi_output() returns: "
                   "|"
                << multi_output << "|" << std::endl;
      outfile << multi_output << std::endl;
    }
  }
  outfile << "\nendmodule\n";

  genVerilogTestBench(input_index, "../Verilog/" + conf.graph_name + "/" +
                                       conf.graph_name + "_tb.v");
}

void SC_sim::Var::genVerilogTestBench(std::vector<int> input_index,
                                      std::string file_name) {
  std::ofstream ofile(file_name);

  ofile << "`timescale 1ns/1ps\n"
           "\n"
           "`define CLOCK_PERIOD 20\n"
           //  "`define SIM_CYCLE 50000\n"
           "\n"
           "module "
        << this->conf.graph_name << "_tb();\n";
  ofile << "\tparameter bit_width = " << conf.bit_width << ";\n";
  // Add 100 for various combination
  ofile << "\tparameter rand_times = 100;\n"
        << "\n";

  ofile << "\treg clk;\n"
           "\treg rst;\n"
           "\n";

  // Registers for inputs of the module
  for (auto i : input_index) {
    ofile << "\treg [bit_width-1:0] b_in" << std::to_string(i) << ";\n";
  }
  ofile << "\twire [bit_width-1:0] b_dest;\n"
           "\n";

  ofile << "\tinteger i = 0;\n"
           "\n";

  // Core Module
  ofile << "\t" << this->conf.graph_name
        << " i1(\n"
           "\t\tclk,\n"
           "\t\trst,\n";
  for (auto i : input_index) {
    ofile << "\t\tb_in" << std::to_string(i) << ",\n";
  }
  ofile << "\t\tb_dest\n"
           "\t);\n"
           "\n";

  // Clock period
  ofile << "\talways begin\n"
           "\t\t#(`CLOCK_PERIOD / 2) clk <= ~clk;\n"
           "\tend\n"
           "\n";

  ofile << "\tinitial begin\n"
           "\t\t$dumpfile(\"circuit.vcd\");\n"
           "\t\t$dumpvars;\n"
           "\n"

           "\t\tclk = 0;\n";

  ofile << "\t\tfor(i = 0; i < rand_times; i = i + 1) begin\n"
        << "\t\t\trst = 1;\n"
        << "\t\t\t#(`CLOCK_PERIOD);\n";

  for (auto i : input_index) {
    ofile << "\t\t\tb_in" << std::to_string(i) << " = $random;\n";
  }
  ofile << "\t\t\trst = 0;\n";
  if (!this->graph.SNGs.empty())
    ofile << "\t\t\t#(`CLOCK_PERIOD * (2 ** bit_width));// give time to "
             "calculate\n";
  ofile << "\t\tend\n"
           "\t\t$finish;\n"
           "\tend\n"
           "\n";

  ofile << "endmodule\n";
}

void SC_sim::Var::find_CI_WCI_in_BC_SC(
    std::vector<std::pair<Node *, Node *>> &CI_pairs,
    std::vector<std::vector<Node *>> &WCI_sets) {
  // Must be used before addCalculate
  if (this->graph.all_nodes.empty())
    this->graph.generate_all_nodes();

  std::map<Node *, std::vector<Node *>> lfsr_sibling_map;

  std::stack<Node *> stack;
  std::stack<VAR_TYPE> type_stack;

  stack.emplace(this->graph.tail);
  type_stack.emplace(this->type);

  while (!stack.empty()) {
    auto *node = stack.top();
    stack.pop();
    auto type = type_stack.top();
    type_stack.pop();

    bool contain_SCTanh = false;
    if (dynamic_cast<SourceNode *>(node) != nullptr)
      continue;

    std::vector<Node *> children;
    std::vector<std::pair<Node *, Node *>>
        CI_result;   // CI pairs for Partial Domain Tree

    switch (type) {
    case BC: {
      children = domain_tree_search_2(node, BC);

      for (auto child : children) {
        stack.emplace(child);
        type_stack.emplace(SC);
      }

      CI_result = find_CI_in_BC(node, children);

      break;
    }

    case SC: {
      children = domain_tree_search_2(node, SC);

      for (auto child : children) {   // sng, SCtanh
        assert(child->inputs.size() == 1);
        stack.emplace(child);
        type_stack.emplace(BC);
        if (dynamic_cast<SCTanhNode *>(child) != nullptr) {
          contain_SCTanh = true;
        }
        lfsr_sibling_map[child] = children;
      }
      // children: sngs; tail: APC or Counter (Destination Node not possible)
      if (contain_SCTanh)
        continue;

      CI_result = find_CI_in_SC(node, children);
      WCI_sets = find_WCI_in_BC_SC(node, children);
      break;
    }
    default:
      break;
    }

    update_CI_pair(CI_pairs, CI_result, node, children, lfsr_sibling_map);
  }

  // exclude CI pair with the same inputs
  std::erase_if(CI_pairs, [](const auto &ci_pair) {
    return ci_pair.first == ci_pair.second;
  });

  // exclude CI pair from WCI set
  std::erase_if(WCI_sets, [CI_pairs](const auto &wci_set) {
    for (auto ci_pair : CI_pairs) {
      if (wci_set[0] == ci_pair.first || wci_set[1] == ci_pair.first ||
          wci_set[2] == ci_pair.first || wci_set[0] == ci_pair.second ||
          wci_set[1] == ci_pair.second || wci_set[2] == ci_pair.second) {
        return true;
      }
    }
    return false;
  });
  std::cout << "Total " << CI_pairs.size() << " CI pairs: " << std::endl;

  std::cout << "Total " << WCI_sets.size() << " WCI sets: " << std::endl;
}

std::vector<Node *> SC_sim::Var::domain_tree_search_2(Node *root,
                                                      VAR_TYPE search_type) {
  std::vector<Node *> res;
  std::stack<Node *> stack;
  stack.emplace(root);

  while (!stack.empty()) {
    auto *node = stack.top();
    stack.pop();
    switch (search_type) {
    case BC:
      if (dynamic_cast<APCNode *>(node) != nullptr ||
          dynamic_cast<CounterNode *>(node) != nullptr ||
          dynamic_cast<SCTanhNode *>(node) != nullptr) {
        for (auto *child_node : node->inputs) {
          if (std::find(res.begin(), res.end(), child_node) == res.end())
            // consider reconvergent circuits. one node can be added multiple
            // times
            res.emplace_back(child_node);
        }
      } else {
        for (auto *child_node : node->inputs) {
          stack.emplace(child_node);
        }
      }

      break;
    case SC:
      // if (dynamic_cast<SCTanhNode *>(node) != nullptr) {
      //   // TODO
      //   // now SC domain search terminate when encountering SCTanh
      //   res.emplace_back(node);
      // }

      if (dynamic_cast<SNGNode *>(node) != nullptr) {
        if (std::find(res.begin(), res.end(), node) == res.end())
          // consider reconvergent circuits. one node can be added multiple
          // times
          res.emplace_back(node);
        // the tail of BC domain can be SNGs, since we do not need to manually
        // search, that is Okay
      } else {
        for (auto *child_node : node->inputs) {
          stack.emplace(child_node);
        }
      }
      break;
    default:
      break;
    }
  }

  return res;
}

std::vector<std::pair<Node *, Node *>>
SC_sim::Var::find_CI_in_BC(Node *tail, std::vector<Node *> inputs) {
  std::vector<std::pair<Node *, Node *>> res;
  for (int i = 0; i < inputs.size(); i++) {
    for (int j = i + 1; j < inputs.size(); j++) {
      res.emplace_back(std::pair<Node *, Node *>(inputs[i], inputs[j]));
    }
  }

  // std::cout << "BC Domain Total " << res.size() << " CI pairs: " <<
  // std::endl; for (auto &i : res) {
  //   std::cout << i.first << " " << i.second << std::endl;
  // }

  return res;
}

std::vector<std::pair<Node *, Node *>>
SC_sim::Var::find_CI_in_SC(Node *tail, std::vector<Node *> inputs) {
  std::vector<std::pair<Node *, Node *>> res;
  std::vector<std::vector<int>> detect_sets(inputs.size());

  for (int i = 0; i < inputs.size(); i++) {
    for (int index_pattern = 0; index_pattern < int(pow(2, inputs.size()));
         index_pattern++) {
      // generate test pattern
      bitstream_list test_pattern = num_to_arr(inputs.size(), index_pattern);

      // std::cout << "test pattern ";
      // for (auto ii : test_pattern) {
      //   std::cout << ii << " ";
      // }
      // std::cout << std::endl;

      // correct
      int correct_out = simulate_CI_in_SC(test_pattern, -1, 0, inputs, tail);

      // detect single stuck-at model for i
      // fault
      int fault_out_i =
          simulate_CI_in_SC(test_pattern, i, !test_pattern[i], inputs, tail);

      if (correct_out != fault_out_i)   // detect
        detect_sets[i].emplace_back(index_pattern);
    }
  }

  for (int i = 0; i < inputs.size(); i++) {
    for (int j = i + 1; j < inputs.size(); j++) {
      std::vector<int> temp = {};
      std::set_intersection(detect_sets[i].begin(), detect_sets[i].end(),
                            detect_sets[j].begin(), detect_sets[j].end(),
                            std::back_inserter(temp));
      // i,j are CI pair
      if (temp.empty()) {
        res.emplace_back(std::pair<Node *, Node *>(inputs[i], inputs[j]));
      }
    }
  }

  // std::cout << "SC Domain Total " << res.size() << " CI pairs: " <<
  // std::endl; for (auto &i : res) {
  //   std::cout << i.first << " " << i.second << std::endl;
  // }

  clear_graph();

  return res;
}

// Return the output value
int SC_sim::Var::simulate_CI_in_SC(std::vector<int> pattern, int index,
                                   int value, std::vector<Node *> inputs,
                                   Node *tail) {
  // int i_acc = 0;
  // for (Node *input : inputs) {
  //   auto temp = dynamic_cast<SNGNode *>(input);
  //   assert(temp != nullptr);
  //   temp->run_CI(index == i_acc ? value : pattern[i_acc]);
  //   i_acc++;
  // }

  for (int i = 0; i < inputs.size(); i++) {
    auto temp = dynamic_cast<SNGNode *>(inputs[i]);
    assert(temp != nullptr);
    temp->run_CI(index == i ? value : pattern[i]);
  }

  return tail->data[0];
}

std::vector<std::vector<Node *>>
SC_sim::Var::find_WCI_in_BC_SC(Node *tail, std::vector<Node *> inputs) {
  // Identical to the paper, we only find weakly CI set of size 3
  std::vector<std::vector<Node *>> res;

  // currently only consider WCI_set in graph with no less than 3 inputs
  if (inputs.size() < 3)
    return res;

  // for all possible set of size 3
  for (int i = 0; i < inputs.size(); i++) {
    for (int j = i + 1; j < inputs.size(); j++) {
      for (int k = j + 1; k < inputs.size(); k++) {
        // bool contain_ci_pair = false;

        // std::cout << "possible WCI set" << inputs[i]->id << inputs[j]->id
        //           << inputs[k]->id << std::endl;

        std::vector<int> cube(inputs.size(), 0);
        cube[i] = -1;
        cube[j] = -1;
        cube[k] = -1;

        bool is_weakly = true;
        // for each cofactor
        for (int cofactor_value = 0;
             cofactor_value < int(pow(2, inputs.size() - 3));
             cofactor_value++) {
          bitstream_list cofactor =
              num_to_arr(inputs.size() - 3, cofactor_value);
          int temp = 0;
          for (int cube_index = 0; cube_index < inputs.size(); cube_index++) {
            if (cube[cube_index] == -1)
              temp--;
            else
              cube[cube_index] = cofactor[cube_index + temp];
          }
          assert(temp == -3);
          if (!test_cofactor_CI_in_BC_SC({i, j, k}, tail, inputs, cube)) {
            is_weakly = false;
            break;
          }
        }
        if (is_weakly) {
          res.push_back({inputs[i], inputs[j], inputs[k]});
        }
      }
    }
  }
  return res;
}

// wci_set: indexes of inputs vector
bool SC_sim::Var::test_cofactor_CI_in_BC_SC(WCI_set wci_set, Node *tail,
                                            std::vector<Node *> inputs,
                                            std::vector<int> cofactor) {
  assert(wci_set.size() == 3);
  std::vector<std::vector<int>> detect_sets(3);
  for (int i = 0; i < 3; i++) {
    for (int index_pattern = 0; index_pattern < 8; index_pattern++) {
      // generate test pattern
      std::vector<int> pattern = cofactor;
      bitstream_list test_pattern = num_to_arr(3, index_pattern);
      pattern[wci_set[0]] = test_pattern[0];
      pattern[wci_set[1]] = test_pattern[1];
      pattern[wci_set[2]] = test_pattern[2];

      // correct
      int correct_out = simulate_WCI_in_BC_SC(pattern, inputs, tail);

      // detect single stuck-at model for i
      pattern[wci_set[i]] = !pattern[wci_set[i]];
      // fault
      int fault_out_i = simulate_WCI_in_BC_SC(pattern, inputs, tail);

      if (correct_out != fault_out_i)   // detect
      {
        detect_sets[i].emplace_back(index_pattern);
      }
    }
  }
  for (int i = 0; i < 3; i++) {
    for (int j = i + 1; j < 3; j++) {
      std::vector<int> temp = {};
      std::set_intersection(detect_sets[i].begin(), detect_sets[i].end(),
                            detect_sets[j].begin(), detect_sets[j].end(),
                            std::back_inserter(temp));
      // i,j are CI pair
      if (temp.empty()) {
        // std::cout << "find CI pair " << wci_set[i] << " " << wci_set[j]
        //           << std::endl;
        // std::cout << "Cofactor ";
        // for (auto &k : cofactor) {
        //   std::cout << k << " ";
        // }
        // std::cout << std::endl;

        return true;
      }
    }
  }
  clear_graph();
  return false;
}

int SC_sim::Var::simulate_WCI_in_BC_SC(std::vector<int> pattern,
                                       std::vector<Node *> inputs, Node *tail) {
  // int i_acc = 0;
  // for (auto &input : inputs) {
  //   auto temp = dynamic_cast<SNGNode *>(input);
  //   assert(temp != nullptr);
  //   temp->run_CI(pattern[i_acc]);
  //   i_acc++;
  // }

  for (int i = 0; i < inputs.size(); i++) {
    auto temp = dynamic_cast<SNGNode *>(inputs[i]);
    assert(temp != nullptr);
    temp->run_CI(pattern[i]);
  }

  return tail->data[0];
}

void SC_sim::Var::update_CI_pair(
    std::vector<std::pair<Node *, Node *>> &CI_pairs,
    std::vector<std::pair<Node *, Node *>> new_CI_pairs, Node *tail,
    const std::vector<Node *> &inputs,
    std::map<Node *, std::vector<Node *>> lfsr_sibling_map) {
  // SourceNode -> LFSR will result in an empty BC domain tree
  if (inputs.empty())
    return;

  // add new CI pairs
  CI_pairs.insert(CI_pairs.end(), new_CI_pairs.begin(), new_CI_pairs.end());

  // update existing relationship
  std::vector<std::pair<Node *, Node *>> temp_CI_pairs;

  for (auto &CI_pair : CI_pairs) {
    // std::cout << "pair updating" << CI_pair.first->id << CI_pair.second->id
    //           << std::endl;

    if (tail == CI_pair.first) {
      for (auto input : inputs) {
        temp_CI_pairs.emplace_back(CI_pair.second, input);
      }
    } else {
      if (tail == CI_pair.second) {
        for (auto input : inputs) {
          temp_CI_pairs.emplace_back(CI_pair.first, input);
        }
      }
    }
  }
  CI_pairs.insert(CI_pairs.end(), temp_CI_pairs.begin(), temp_CI_pairs.end());

  if (dynamic_cast<SNGNode *>(tail) == nullptr) {
    // keep it if it's a LFSR
    std::erase_if(CI_pairs, [tail](const auto &ci_pair) {
      return tail == ci_pair.first || tail == ci_pair.second;
    });
  }

  if (lfsr_sibling_map.contains(tail)) {
    // tail is LFSR -> current is BC domain
    for (auto &lfsr : lfsr_sibling_map[tail]) {
      for (auto &input : inputs) {
        // add new CI relationship if it doesn't exist
        if ((std::find(CI_pairs.begin(), CI_pairs.end(),
                       std::pair<Node *, Node *>(input, lfsr)) ==
             CI_pairs.end()) &&
            (std::find(CI_pairs.begin(), CI_pairs.end(),
                       std::pair<Node *, Node *>(lfsr, input)) ==
             CI_pairs.end())) {
          CI_pairs.emplace_back(std::pair<Node *, Node *>(input, lfsr));
        }
      }
    }
  }
}

std::vector<std::vector<std::vector<Node *>>>
SC_sim::Var::find_clique_partition(
    std::vector<std::pair<Node *, Node *>> CI_set) {
  if (this->graph.all_nodes.empty())
    this->graph.generate_all_nodes();

  // std::vector<int> sng_indexs;
  // for (auto node : this->graph.all_nodes) {
  //   if (dynamic_cast<SNGNode *>(node) != nullptr) {
  //     sng_indexs.emplace_back(this->graph.node_id_dict[node]);
  //   }
  // }
  std::vector<std::vector<std::vector<Node *>>> possible_cliques;

  // srand(time(nullptr));
  for (int i = 0; i < this->conf.rand_clique_partition_times; i++) {
    std::vector<std::vector<Node *>> cliqueList = min_clique_cover(CI_set);
    possible_cliques.push_back(cliqueList);
    // this->conf.cliquePartitionList.emplace_back(cliqueList);

    // std::cout << "Random: " << i << std::endl;
    // for (const auto &clique : cliqueList) {
    //   for (const auto &c : clique) {
    //     std::cout << c->id << " ";
    //   }
    //   std::cout << std::endl;
    // }
  }
  return possible_cliques;
}

std::vector<std::vector<Node *>>
SC_sim::Var::min_clique_cover(std::vector<std::pair<Node *, Node *>> edge_set) {
  std::vector<Node *> inputs;
  for (auto &sng : this->graph.SNGs) {
    inputs.push_back(sng);
  }

  std::vector<std::vector<Node *>> clique_list;
  std::map<Node *, std::set<Node *>> v_e_map = get_vertex(edge_set);

  std::vector<std::pair<Node *, int>> v_degree(inputs.size());
  for (int i = 0; i < v_degree.size(); i++)
    v_degree[i] = std::pair<Node *, int>(inputs[i], v_e_map[inputs[i]].size());

  // sort based on degree
  std::sort(v_degree.begin(), v_degree.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  for (const auto &input : inputs) {
    // try to put a new vertice into available clique
    std::vector<int> available_cliqiues = {};
    for (int clique_index = 0; clique_index < clique_list.size();
         clique_index++) {
      bool fit_in = true;
      // a vertice can be added to a clique if it has edges with all vertices
      // in the clique
      for (auto &vert : clique_list[clique_index]) {
        if (std::find(v_e_map[input].begin(), v_e_map[input].end(), vert) ==
            v_e_map[input].end()) {
          // this vertice in the clique is not found in the edges of the new
          // vertice we plan to add
          fit_in = false;
          break;
        }
      }
      if (fit_in) {
        available_cliqiues.emplace_back(clique_index);
      }
    }

    if (available_cliqiues.empty()) {
      clique_list.push_back({input});
    } else {
      clique_list[available_cliqiues[rand() % available_cliqiues.size()]]
          .emplace_back(input);
    }
  }
  return clique_list;
}

std::map<Node *, std::set<Node *>>
SC_sim::Var::get_vertex(std::vector<std::pair<Node *, Node *>> edge_set) {
  std::map<Node *, std::set<Node *>> v_e_map;
  for (auto &i : edge_set) {
    v_e_map[i.first].insert(i.second);
    v_e_map[i.second].insert(i.first);
  }
  return v_e_map;
}

void SC_sim::Var::set_WCI_in_BC_SC(
    const std::vector<std::vector<Node *>> &wci_sets) {
  // WCI
  std::vector<std::vector<Node *>> used_wci_sets = {};

  for (const auto &wci_set : wci_sets) {
    bool wci_set_overlap = false;
    for (auto &used_wci_set : used_wci_sets) {
      auto ip = std::find_first_of(wci_set.begin(), wci_set.end(),
                                   used_wci_set.begin(), used_wci_set.end());
      if (ip != wci_set.end()) {
        wci_set_overlap = true;
        break;
      }
    }
    if (!wci_set_overlap) {
      used_wci_sets.emplace_back(wci_set);
      assert(wci_set.size() == 3);
      auto first = dynamic_cast<SNGNode *>(wci_set[0]);
      assert(first != nullptr);
      auto second = dynamic_cast<SNGNode *>(wci_set[1]);
      assert(second != nullptr);
      auto third = dynamic_cast<SNGNode *>(wci_set[2]);
      assert(third != nullptr);
      assert(third->master.empty());
      third->master = {first, second};
    }
  }
  // std::cout << "The following WCI set is used to reduced area" << std::endl;
  // for (auto wci_set : used_wci_sets) {
  //   std::cout << wci_set[0] << " " << wci_set[1] << " " << wci_set[2]
  //             << std::endl;
  // }
}

void SC_sim::Var::set_CI_in_BC_SC(
    const std::vector<std::vector<Node *>> &clique_list) {
  if (this->graph.all_nodes.empty())
    this->graph.generate_all_nodes();
  //
  // std::unordered_map<Node *, int> node_id_dict;
  // for (size_t i = 0; i < this->graph.all_nodes.size(); i++) {
  //   node_id_dict.emplace(this->graph.all_nodes[i], i);
  // }
  //
  // std::vector<int> sng_indexs;
  // for (auto node : this->graph.all_nodes) {
  //   if (dynamic_cast<SNGNode *>(node) != nullptr) {
  //     sng_indexs.emplace_back(node_id_dict[node]);
  //   }
  // }

  for (auto clique : clique_list) {
    if (clique.size() > 1) {
      auto first = dynamic_cast<SNGNode *>(clique[0]);
      first->master.clear();
      for (int remain = 1; remain < clique.size(); remain++) {
        auto second = dynamic_cast<SNGNode *>(clique[remain]);
        assert(second != nullptr);
        second->master = {first};
      }
    }
  }
  this->clear_graph();
}

std::pair<double, std::string> SC_sim::Var::simulate(
    std::vector<std::vector<std::vector<Node *>>> possible_cliques) {
  if (possible_cliques.empty()) {
    return this->simulate();
  }

  srand(this->conf.seed);

  optimize();

  if (this->graph.all_nodes.empty())
    this->graph.generate_all_nodes();

  for (const auto node : this->graph.all_nodes) {
    node->clear();
  }

  // Place constant source nodes at the beginning
  std::vector<SourceNode *> new_inputs;
  int non_const_sng_index = 0;
  for (size_t i = 0; i < this->graph.inputs.size(); i++) {
    if (dynamic_cast<ConstantSourceNode *>(this->graph.inputs[i]) != NULL) {
      new_inputs.insert(new_inputs.begin(), this->graph.inputs[i]);
      non_const_sng_index++;
    } else {
      new_inputs.push_back(this->graph.inputs[i]);
    }
  }
  this->graph.inputs = new_inputs;

  for (size_t i = 0; i < this->graph.inputs.size() - 1; i++) {
    assert(this->graph.inputs[i]->get_next_node() == nullptr);
    this->graph.inputs[i]->set_next_node(this->graph.inputs[i + 1]);
  }

  assert(this->graph.inputs[this->graph.inputs.size() - 1]->get_next_node() ==
         nullptr);

  switch (conf.simType) {
  case rand_sim:
    this->graph.inputs[non_const_sng_index]->init(
        {std::to_string(conf.rand_sim_times)});
    break;
  case user_def_sim:
    this->graph.inputs[non_const_sng_index]->init({});
  default:
    break;
  }

  std::cout << "------------------------------ simulation start "
               "------------------------------"
            << std::endl;
  std::cout << conf << std::endl;

  auto time_start = std::chrono::high_resolution_clock::now();

  std::unordered_map<CalculateNode *, double> sums;
  for (const auto &cal : this->graph.calculates) {
    sums[cal] = 0;
  }

  std::vector<double> MAEs;
  std::vector<std::string> confs;

  int clique_times =
      std::min(conf.rand_clique_partition_times, (int) possible_cliques.size());

  // (MAE, (clique conf, RNS conf))
  std::vector<std::pair<double, std::pair<std::string, std::string>>> results;

  std::set<int> simulated_index_set;
  for (int i = 0; i < clique_times; ++i) {
    std::string clique_conf;

    int rand_index;
    if (clique_times == possible_cliques.size()) {
      rand_index = i;
    } else {
      do {
        rand_index = rand() % possible_cliques.size();
      } while (simulated_index_set.contains(rand_index));
    }

    set_CI_in_BC_SC(possible_cliques[rand_index]);

    std::stringstream clique_partition_buffer;
    for (auto clique : possible_cliques[rand_index]) {
      for (auto node : clique) {
        clique_partition_buffer << node->id << " ";
      }
      clique_partition_buffer << std::endl;
    }

    clique_conf = clique_partition_buffer.str();
    // std::cout << i << "th simulation with following clique partitioning"
    //           << std::endl;
    // std::cout << clique_conf;

    std::pair<double, std::string> ret = simulate_method(conf.rand_times);

    results.push_back(std::pair<double, std::pair<std::string, std::string>>(
        ret.first,
        std::pair<std::string, std::string>(clique_conf, ret.second)));
  }

  std::sort(results.begin(), results.end(),
            [](const auto &a, const auto &b) { return a.first < b.first; });

  auto time_end = std::chrono::high_resolution_clock::now();

  auto time =
      std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start);

  std::cout << std::endl;
  std::cout << "Time cost: " << time.count() << " seconds." << std::endl;

  std::cout << "------------------------------ simulation end   "
               "------------------------------"
            << std::endl;

  return {results[0].first, results[0].second.second};
}

std::pair<double, std::string>
SC_sim::Var::simulate_conf(std::string RNS_config) {
  srand(this->conf.seed);

  optimize();

  if (this->graph.all_nodes.empty())
    this->graph.generate_all_nodes();

  for (const auto node : this->graph.all_nodes) {
    node->clear();
  }

  // Place constant source nodes at the beginning
  std::vector<SourceNode *> new_inputs;
  int non_const_sng_index = 0;
  for (size_t i = 0; i < this->graph.inputs.size(); i++) {
    if (dynamic_cast<ConstantSourceNode *>(this->graph.inputs[i]) != NULL) {
      new_inputs.insert(new_inputs.begin(), this->graph.inputs[i]);
      non_const_sng_index++;
    } else {
      new_inputs.push_back(this->graph.inputs[i]);
    }
  }
  this->graph.inputs = new_inputs;

  for (size_t i = 0; i < this->graph.inputs.size() - 1; i++) {
    assert(this->graph.inputs[i]->get_next_node() == nullptr);
    this->graph.inputs[i]->set_next_node(this->graph.inputs[i + 1]);
  }

  assert(this->graph.inputs[this->graph.inputs.size() - 1]->get_next_node() ==
         nullptr);

  switch (conf.simType) {
  case rand_sim:
    this->graph.inputs[non_const_sng_index]->init(
        {std::to_string(conf.rand_sim_times)});
    break;
  case user_def_sim:
    this->graph.inputs[non_const_sng_index]->init({});
  default:
    break;
  }

  // Config RNSs
  config_by_str(RNS_config);

  std::cout << "------------------------------ simulation start "
               "------------------------------"
            << std::endl;
  std::cout << conf << std::endl;

  auto time_start = std::chrono::high_resolution_clock::now();

  // Start simulation
  std::pair<double, std::string> ret = simulate_once();

  auto time_end = std::chrono::high_resolution_clock::now();

  auto time =
      std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start);

  std::cout << ret.second << std::endl;

  std::cout << std::endl;
  std::cout << "Time cost: " << time.count() << " seconds." << std::endl;

  std::cout << "------------------------------ simulation end   "
               "------------------------------"
            << std::endl;

  return ret;
}