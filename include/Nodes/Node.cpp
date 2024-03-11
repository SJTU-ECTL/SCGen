#include "Node.h"

#include <sstream>

int Node::id_pool = 0;

Config::Config() {
  inserting_zero = 0;
  leq = 0;
  bit_width = 3;
  rand_times = 1000;
  optimize = 0;
  search_method = "RANDOM";
  rand_till_converge = false;

  // in this way, the random seed will be decided at compile time, not
  // runtime.
  srand(time(NULL));
  static long seed_rand = rand();
  seed = seed_rand;

  simType = full_sim;
  rand_sim_times = 10000;
}

const std::string Config::codegen_str(std::string name) const {
  std::stringstream str;
  str << "  " << name << ".inserting_zero = " << inserting_zero << ";"
      << std::endl;
  str << "  " << name << ".leq = " << leq << ";" << std::endl;
  str << "  " << name << ".bit_width = " << bit_width << ";" << std::endl;
  str << "  " << name << ".rand_times = " << rand_times << ";" << std::endl;
  str << "  " << name << ".graph_name = \"" << graph_name << "\";" << std::endl;
  str << "  " << name << ".optimize = " << optimize << ";" << std::endl;
  str << "  " << name << ".seed = " << seed << ";" << std::endl;
  str << "  " << name << ".search_method = \"" << search_method << "\";"
      << std::endl;
  return str.str();
}

void Config::set_value(std::string name, std::string value) {
  if (name == "bitwidth") {
    bit_width = stoi(value);
  } else if (name == "inserting_zero") {
    inserting_zero = stoi(value);
  } else if (name == "leq") {
    leq = stoi(value);
  } else if (name == "fn") {
    fn = value;
  } else if (name == "rand_times") {
    rand_times = stoi(value);
  } else if (name == "rand_till_converge") {
    rand_till_converge = (value.compare("true") == 0);
  } else if (name == "optimize") {
    optimize = stoi(value);
  } else if (name == "seed") {
    if (value == "random") {
      // in this way, the random seed will be decided at compile time, not
      // runtime.
      srand(time(NULL));
      static long seed_rand = rand();
      seed = seed_rand;
    } else {
      seed = stol(value);
    }
  } else if (name == "search_method") {
    search_method = value;
  }
}

std::ostream &operator<<(std::ostream &os, const Config &conf) {
  os << "Config of graph " << conf.graph_name << ":" << std::endl;
  os << "inserting_zero " << conf.inserting_zero << ", leq " << conf.leq
     << ", bit_width " << conf.bit_width << ", rand_times " << conf.rand_times
     << ", optimize " << conf.optimize << ", seed " << conf.seed
     << ", rand_till_converge " << conf.rand_till_converge << ", search_method "
     << conf.search_method << "." << std::endl;

  return os;
}

void Node::set_in_node(int index, Node *node) {
  // assert(index < input_num);
  if (index < 0)
    return;
  if (index >= input_num) {
    input_num = index + 1;
    while (inputs.size() != input_num) {
      inputs.push_back(nullptr);
    }
  }
  if (inputs[index] != nullptr) {
    // std::cerr << "(info) override one inputs in a Node: " << std::endl;
  }
  inputs[index] = node;
}
void Node::set_out_node(int index, Node *node) {
  // assert(index < output_num);
  if (index < 0)
    return;
  if (index >= output_num) {
    output_num = index + 1;
    while (outputs.size() != output_num) {
      outputs.push_back(nullptr);
    }
  }
  if (outputs[index] != nullptr) {
    // std::cerr << "(info) override one outputs in a Node: " << std::endl;
  }
  outputs[index] = node;
}

int Node::find_in_node(Node *node) { return Node::find_in_node(node, this); }
int Node::find_out_node(Node *node) { return Node::find_out_node(this, node); }

int Node::find_in_node(Node *first, Node *second) {
  int index = 0;
  for (const auto in : second->inputs) {
    if (first == in) {
      return index;
    }
    index++;
  }
  return -1;
}
int Node::find_out_node(Node *first, Node *second) {
  int index = 0;
  for (const auto out : first->outputs) {
    if (second == out) {
      return index;
    }
    index++;
  }
  return -1;
}

void Node::connect(int out_index, Node *node, int in_index) {
  node->set_in_node(in_index, this);
  this->set_out_node(out_index, node);
}

void Node::notify() {
  for (const auto &out : outputs)
    out->run();
}

void Node::notify_reset_zero() {
  for (const auto &out : outputs)
    out->reset_zero();
}

void Node::notify_simple() {
  for (const auto &out : outputs)
    out->simple_run();
}

bool Node::is_ready() {
  for (const auto &in : inputs)
    if (!in->ready)
      return false;
  return true;
}

void Node::clear_ready() {
  ready = false;
  for (const auto &out : outputs)
    out->clear_ready();
}
