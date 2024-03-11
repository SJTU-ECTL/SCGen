#include "Modules/DAG.h"

void SC_sim::DAG::borrow(const DAG &second) {
  this->inputs = second.inputs;
  this->outputs = second.outputs;
  this->calculates = second.calculates;
  this->all_nodes = second.all_nodes;

  this->tail = second.tail;
  this->expression = second.expression;
  this->SNGs = second.SNGs;
}

void SC_sim::DAG::operator=(const DAG &second) {
  // Clear current DAG
  this->inputs.clear();
  this->outputs.clear();
  this->calculates.clear();
  this->all_nodes.clear();
  this->tail = nullptr;
  this->SNGs.clear();

  // map original node->new node
  std::unordered_map<Node *, Node *> map;

  std::vector<Node *> orig_nodes = second.get_all_nodes();

  for (const auto &node : orig_nodes) {
    Node *node_new = node->copy();
    map[node] = node_new;
  }

  // Restore SubcircuitNode
  for (const auto &[_, node] : map) {
    SubcircuitNode *subcircuit_node = dynamic_cast<SubcircuitNode *>(node);
    if (subcircuit_node) {
      // for (int i = 0; i < subcircuit_node->starts.size(); i++) {
      //   subcircuit_node->starts[i] = map[subcircuit_node->starts[i]];
      // }
      // for (int i = 0; i < subcircuit_node->ends.size(); i++) {
      //   subcircuit_node->ends[i] = map[subcircuit_node->ends[i]];
      // }
    }
  }

  for (const auto &node : orig_nodes) {
    for (const auto &out : node->outputs) {
      map[node]->connect(node->find_out_node(node, out), map[out],
                         out->find_in_node(node, out));
    }
  }

  // DAG restore
  for (const auto &input : second.inputs) {
    this->inputs.push_back(dynamic_cast<SourceNode *>(map[input]));
  }
  for (const auto &output : second.outputs) {
    this->outputs.push_back(dynamic_cast<DestNode *>(map[output]));
  }
  for (const auto &calculate : second.calculates) {
    this->calculates.push_back(dynamic_cast<CalculateNode *>(map[calculate]));
  }
  this->tail = map[second.tail];
  this->expression = second.expression;

  this->generate_all_nodes();
}
std::vector<Node *> SC_sim::DAG::get_all_nodes() const {
  std::vector<Node *> ret = {};
  std::vector<Node *> queue = {};
  for (const auto &in : this->inputs)
    queue.push_back(in);

  while (!queue.empty()) {
    Node *now = queue.front();
    queue.erase(queue.begin());
    if (std::find(ret.begin(), ret.end(), now) != ret.end()) {
      // find
      continue;
    }

    for (auto const &out : now->outputs)
      queue.push_back(out);

    ret.emplace_back(now);
  }

  return ret;
}

void SC_sim::DAG::generate_all_nodes() {
  this->all_nodes.clear();
  std::vector<Node *> queue = {};
  for (const auto &in : this->inputs)
    queue.push_back(in);

  int i = 0;
  while (!queue.empty()) {
    Node *now = queue.front();
    queue.erase(queue.begin());
    if (std::find(this->all_nodes.begin(), this->all_nodes.end(), now) !=
        this->all_nodes.end()) {
      // find
      continue;
    }

    for (auto const &out : now->outputs)
      queue.push_back(out);

    this->all_nodes.emplace_back(now);
    this->node_id_dict[now] = i++;
  }

  // Add SNG nodes
  for (const auto &node : this->all_nodes) {
    if (SNGNode *temp = dynamic_cast<SNGNode *>(node)) {
      this->SNGs.push_back(temp);
    }
    if (SubcircuitNode *temp = dynamic_cast<SubcircuitNode *>(node)) {
      for (const auto &start : temp->starts) {
        this->SNGs.push_back(dynamic_cast<SNGNode *>(start));
      }
    }
  }
}