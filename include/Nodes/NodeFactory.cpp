#include "NodeFactory.h"

template <typename T>
Node *createInstance(const std::pair<int, int> &in_out_num,
                     const Config &conf) {
  return new T(in_out_num.first, in_out_num.second, conf);
}

Node *NodeFactory::create(std::string name,
                          const std::pair<int, int> &in_out_num,
                          const std::vector<std::string> &extra,
                          const Config &conf) {
  if (!map.contains(name)) {
    std::cout << "No Node type found. Maybe not registered correctly in "
                 "NodeFactory.h?"
              << std::endl;
    return nullptr;
  }

  Node *node = map[name](in_out_num, conf);
  node->init(extra);
  return node;
}