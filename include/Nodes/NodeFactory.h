/**
 * @brief
 *     The place to register newly added nodes, mapping the name of the node to
 *         its class definition. Used in CodeGen.
 */

#ifndef NODEFACTORY_H
#define NODEFACTORY_H

#include "BinaryNode.h"
#include "CalculateNode.h"
#include "FunctionNode.h"
#include "Node.h"
#include "Nodes/DisplayNode.h"
#include "OtherNode.h"
#include "SNGNode.h"
#include "SourceNode.h"
#include <unordered_map>

template <typename T>
Node *createInstance(const std::pair<int, int> &in_out_num, const Config &conf);

class NodeFactory {
private:
  static inline std::unordered_map<
      std::string, Node *(*) (const std::pair<int, int> &, const Config &)>
      map{{"AddNode", &createInstance<AddNode>},
          {"AndNode", &createInstance<AndNode>},
          {"OrNode", &createInstance<OrNode>},
          {"APCNode", &createInstance<APCNode>},
          {"AverageNode", &createInstance<AverageNode>},
          {"BinaryAbsSubNode", &createInstance<BinaryAbsSubNode>},
          {"BinaryAddNode", &createInstance<BinaryAddNode>},
          {"BinaryDestNode", &createInstance<BinaryDestNode>},
          {"BinaryMulNode", &createInstance<BinaryMulNode>},
          {"BinaryNormAddNode", &createInstance<BinaryNormAddNode>},
          {"BinarySaturAddNode", &createInstance<BinarySaturAddNode>},
          {"BinarySumNode", &createInstance<BinarySumNode>},
          {"BinaryTanhNode", &createInstance<BinaryTanhNode>},
          {"BinomialNode", &createInstance<BinomialNode>},
          {"ConstantSourceNode", &createInstance<ConstantSourceNode>},
          {"CounterNode", &createInstance<CounterNode>},
          {"DFFNode", &createInstance<DFFNode>},
          {"DestNode", &createInstance<DestNode>},
          {"DisplayNode", &createInstance<DisplayNode>},
          {"FSMXKNode", &createInstance<FSMXKNode>},
          {"HypergeometricNode", &createInstance<HypergeometricNode>},
          {"LFSRNode", &createInstance<LFSRNode>},
          {"SobolNode", &createInstance<SobolNode>},
          {"MAENode", &createInstance<MAENode>},
          {"MinNode", &createInstance<MinNode>},
          {"MSENode", &createInstance<MSENode>},
          {"NandNode", &createInstance<NandNode>},
          {"NNode", &createInstance<NNode>},
          {"RandomSourceNode", &createInstance<RandomSourceNode>},
          {"SCTanhNode", &createInstance<SCTanhNode>},
          {"SourceNode", &createInstance<SourceNode>},
          {"UserDefSourceNode", &createInstance<UserDefSourceNode>},
          {"XorNode", &createInstance<XorNode>},
          {"XnorNode", &createInstance<XnorNode>}};

public:
  static Node *create(std::string name, const std::pair<int, int> &in_out_num,
                      const std::vector<std::string> &extra,
                      const Config &conf);
};

#endif