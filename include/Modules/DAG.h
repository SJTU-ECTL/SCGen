#ifndef DAG_H
#define DAG_H

#include "Modules/Expression.h"
#include "Nodes/OptimizeNode.h"
#include "Nodes/OtherNode.h"
#include "Nodes/SNGNode.h"
#include "Nodes/SourceNode.h"

namespace SC_sim {

// TODO: free memory of nodes.
class DAG {
public:
  std::vector<SourceNode *> inputs;
  std::vector<DestNode *> outputs;
  std::vector<CalculateNode *> calculates;
  std::vector<Node *> all_nodes;
  // TODO: consider the case of DAG copy
  // node_id_dict is not used any more, use the id of each node instead
  std::unordered_map<Node *, int> node_id_dict;

  Node *tail;
  Expression expression;

  std::vector<SNGNode *> SNGs;

  void borrow(const DAG &second);

  // Modify the all_nodes of second DAG
  void operator=(const DAG &second);

  std::vector<Node *> get_all_nodes() const;

  void generate_all_nodes();
};
}   // namespace SC_sim

#endif