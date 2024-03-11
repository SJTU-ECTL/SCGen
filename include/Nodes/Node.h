/**
 * @brief
 *     The base class of each node in the simulation framework. All specific
 *         nodes are derived from this abstract class.
 *     The definition of global configurations are also described here.
 */

#ifndef NODE_H
#define NODE_H

#include "Headers.h"
#include "Utils/Utils_IO.h"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>

enum OPTIMIZE {
  NONE = 0,
  INCREMENTAL = 1 << 0,       // 1
  SUBCIRCUIT = 1 << 1,        // 2
  RUNNING_MINIMAL = 1 << 2,   // 4
  LOOKUPTABLE = 1 << 3,       // 8
};

enum Sim_type {
  full_sim,
  rand_sim,
  user_def_sim,
};

enum SCType {
  unipolar,
  bipolar,
};

class Node;

class Config {
public:
  bool inserting_zero;
  bool leq;
  int bit_width;
  int rand_times;
  int optimize;
  bool rand_till_converge;
  long seed;
  Sim_type simType;
  int rand_sim_times;

  int rand_clique_partition_times = 1;
  std::vector<CliqueList> cliquePartitionList;

  std::string search_method;

  std::string graph_name;
  std::string fn;

  SCType scType = unipolar;

  Config();
  Config(std::string graph_name_, int bit_width_, Sim_type sim_type_,
         int optimize_ = 0, int rand_times_ = 5, int seed_ = 1,
         int rand_sim_times_ = 10) {
    graph_name = graph_name_;
    bit_width = bit_width_;
    simType = sim_type_;
    optimize = optimize_;
    rand_times = rand_times_;
    seed = seed_;
    rand_sim_times = rand_sim_times_;
    inserting_zero = false;
    leq = false;
  }

  void set_value(std::string name, std::string value);

  /**
   * @brief Used to generate initialization codes for a Config instance `name`
   *
   * @param name the name of the Config instance
   * @return const std::string the codes without last `\n`.
   */
  const std::string codegen_str(std::string name) const;

  friend std::ostream &operator<<(std::ostream &os, const Config &conf);
};

class Node {
public:
  std::vector<Node *> inputs;
  std::vector<Node *> outputs;
  int input_num;
  int output_num;
  bool ready = false;
  bool in_sc_core = true;
  const Config conf;

  static int id_pool;
  int id;

  std::vector<std::string> extra;

  virtual std::string label() = 0;

  friend class SC;

  friend class LookupTableNode;
  friend class SubcircuitNode;

public:
  Node &operator=(Node &t) { return *this; }
  // Consider using std::bitset here?
  // std::bitset<4> data;
  std::vector<int> data;

  // Used for optimization
  std::unordered_set<int> update;

  Node() = delete;
  Node(int _input_num, int _output_num, const Config &_conf)
      : inputs(std::vector<Node *>(_input_num)),
        outputs(std::vector<Node *>(_output_num)), input_num(_input_num),
        output_num(_output_num), conf(_conf) {
    id = id_pool;
    id_pool++;
    // std::cout << id << std::endl;
  }

  virtual ~Node(){};

  // Return the copy of current node, should be overwritten by subclasses.
  virtual Node *copy() = 0;
  // {
  //   std::cout << "default copy(): return null" << std::endl;
  //   return nullptr;
  // };

  /**
   * @brief Used when some nodes have extra information when initialization,
   * e.g., the constant value of a ConstantSourceNode.
   * For now it is only used in CodeGen
   * @param extra
   */
  virtual void init(const std::vector<std::string> &extra){};

  std::vector<std::string> get_extra() { return extra; }

  void set_in_node(int index, Node *node);
  void set_out_node(int index, Node *node);
  static int find_in_node(Node *first, Node *second);
  static int find_out_node(Node *first, Node *second);
  int find_in_node(Node *node);
  int find_out_node(Node *node);
  void connect(int out_index, Node *node, int in_index);

  /**
   * @brief Used to generate a Verilog instance and or definition.
   */
  virtual std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "\t" + label() + " #(.bit_width(bit_width)) " + label() + "_" +
           std::to_string(node_index_map[this]) + "(\n" +
           verilog_codegen_io_str(node_index_map) + " );\n";
  }

  std::string verilog_wire_name(Node *first, Node *second) {
    return "wire_" + first->label() + std::to_string(first->id) + "_" +
           second->label() + std::to_string(second->id);
  }
  virtual std::string
  verilog_codegen_multi_output(std::unordered_map<Node *, int> node_index_map) {
    std::string res = "";
    Node *first;
    int index = 0;
    for (auto &i : outputs) {
      if (i->verilog_has_instance()) {
        // exclude Calculate Node
        if (index == 0) {
          index++;
          first = i;
        } else {
          res += "\tassign wire_" + this->label() + std::to_string(this->id) +
                 "_" + i->label() + std::to_string(i->id) + " = wire_" +
                 this->label() + std::to_string(this->id) + "_" +
                 first->label() + std::to_string(first->id) + ";\n";
        }
      }
    }
    return res;
  }

  virtual bool verilog_has_instance() { return true; }
  virtual std::string verilog_module_init_str() { return ""; };

  virtual std::string
  verilog_codegen_io_str(std::unordered_map<Node *, int> node_index_map) {
    std::string input_str;
    for (auto &i : inputs) {
      if (i->verilog_has_instance())   // ensure Node such as CalculateNode is
                                       // not considered
        input_str += "\t\twire_" + i->label() + std::to_string(i->id) + "_" +
                     this->label() + std::to_string(this->id) + ",\n";
    }
    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (i->verilog_has_instance()) {
        input_str += "\t\twire_" + this->label() + std::to_string(this->id) +
                     "_" + i->label() + std::to_string(i->id);
        break;
      }
    }
    return input_str;
  }

  /**
   * @brief Used to generate initialization codes for a specific instance.
   * The name of the instance will be filled with `<-name->`.
   */

  virtual std::string codegen_str() = 0;

  /**
   * @brief Virtual function run(), allows each node do their specified
   * operation.
   * If a node has inputs, check is_ready() first.
   * Set ready to true when finished.
   * If a node has outputs, use notify() to tell its outputs.
   * Template:
   *    void run() {
   *      if (ready || !is_ready())
   *        return;
   *
   *      Do operations here...
   *
   *      ready = true;
   *      notify();
   *    }
   */
  virtual void run() = 0;

  virtual void simple_run(){};

  // Used to reset the condition of the circuits before each random simulation
  virtual void reset(){};

  /**
   * @brief Used for optimization 1: initialize the circuits to zero state, and
   * after that we can use differential increment
   */
  virtual void reset_zero(){};

  // Used for CalculateNode to be cleared when simulate() is invoked.
  virtual void clear(){};

  /**
   * @brief Notify nodes in outputs to run.
   */
  void notify();

  /**
   * @brief Used along with reset_zero()
   */
  void notify_reset_zero();

  /**
   * @brief Used for lookup table generation
   *
   */
  void notify_simple();

  /**
   * @brief Check whether all nodes in inputs are ready
   */
  bool is_ready();

  /**
   * @brief Each source node should call this function.
   */
  void clear_ready();

  std::string get_label() { return label(); }

  /**
   * @brief Get the config of SNGNode as string.
   */
  virtual std::string get_config_str() { return ""; };

  virtual void finish(){};
};

#endif