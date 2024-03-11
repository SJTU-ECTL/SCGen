#ifndef VAR_H
#define VAR_H

#include "Agent.h"
#include "CodeGen/CodeGen.h"
#include "DAG.h"
#include "Expression.h"
#include "Modules/SC_core.h"
#include "Modules/SNG.h"
#include "Modules/Sobol.h"
#include "Nodes/BinaryNode.h"
#include "Nodes/CalculateNode.h"
#include "Nodes/FunctionNode.h"
#include "Nodes/Node.h"
#include "Nodes/OptimizeNode.h"
#include "Nodes/OtherNode.h"
#include "Nodes/SNGNode.h"
#include "Nodes/SourceNode.h"
#include "Op.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <opencv2/imgproc.hpp>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

namespace SC_sim {

enum SNG_TYPE { LFSR_SNG, SOBOL_SNG };

enum VAR_TYPE { SC, BC, UNINITIALIZE };
class Var {
public:
  enum VAR_TYPE type;
  class DAG graph;
  Config conf;

  Var() { this->type = UNINITIALIZE; }

  // Used when overloading happens, combine inputs of two operands inputs of
  // the result
  Var(const std::vector<Var> &seconds);

  Var(const Var &a, const Var &b);

  int get_input_index(int id);

  // User should use this function to reorder the essential input, in order to
  // set correct calculate function
  void reorderInput(std::vector<Var> input_seq);

  void customUserInput(const std::string &input_file, int height, int width,
                       std::vector<Var> input_seq);

  void customUserInput(const std::string &input_file, int size,
                       std::vector<Var> input_seq);

  Var BC_add(const Var &second);
  Var BC_mult(const Var &second);

  Var SC_add(const Var &second);

  static Var BC_to_SC(const Var &second, const Var *copyFrom = NULL);
  static Var SC_to_BC(const Var &second);

public:
  static Var imageVar(int kernel_width, int kernel_height, int pos_x, int pos_y,
                      const std::string &image_file, VAR_TYPE type,
                      const Config &_conf,
                      std::vector<std::string> extra = {}) {
    Var ret;
    ret.type = type;
    ret.conf = _conf;

    auto *in_node = new ImageSourceNode(0, 1, _conf);
    in_node->init({std::to_string(kernel_width), std::to_string(kernel_height),
                   std::to_string(pos_x), std::to_string(pos_y), image_file});

    SNGNode *sng = new LFSRNode(1, 0, _conf);
    sng->init(extra);
    in_node->connect(0, sng, 0);

    ret.graph.inputs.push_back(in_node);
    ret.graph.expression.add_symbol("x_" + std::to_string(pos_x) +
                                    std::to_string(pos_y));

    ret.graph.tail = sng;

    return ret;
  }

  static Var constantVar(float num, VAR_TYPE type, const Config &_conf,
                         const SNG_TYPE &_sng_type = LFSR_SNG,
                         Var *copyFrom = NULL,
                         std::vector<std::string> extra = {});

  static Var new_SC(const Config &_conf, const SNG_TYPE &type = LFSR_SNG,
                    const Var *copyFrom = nullptr,
                    const GiNaC::symbol &_symbol = GiNaC::symbol("temp"));

  static Var new_BC(const Config &_conf);

  Var(const VAR_TYPE &_type, const Config &_conf,
      const SNG_TYPE &_sng_type = LFSR_SNG, const Var *copyFrom = nullptr,
      const GiNaC::symbol &_symbol = GiNaC::symbol("temp"));

  Var(const std::string &type_str, const Config &_conf);

  Var(enum VAR_TYPE _type, Config _conf, const std::string &file_path);

  Var(const std::string &type_str, Config _conf, std::string file_path);

  void operator=(const Var &second);

  Var copy();

  Var operator*(const Var &second);

  Var to_BC();

  Var to_SC();

  static Var apc(std::vector<Var> seconds);

  static Var BinarySum(std::vector<Var> seconds);

  Var operator+(const Var &second);

  Var abs_sub(const Var &second);

  static Var Randomizer(const Var &second, const Var *copyFrom = NULL);

  static Var Counter(const Var &second);

  static Var sum(const std::vector<Var> &seconds);

  static Var pc(const std::vector<Var> &seconds);

  friend Var operator-(int value, const Var &second);

  void reset();

  void finish();

  void addImageDestNode(int kernel_width, int kernel_height, int pos_x,
                        int pos_y, const std::string &image_file, VAR_TYPE type,
                        const Config &_conf) {
    DestNode *out_node = new ImageDestNode(1, 0, _conf);
    out_node->init({std::to_string(kernel_width), std::to_string(kernel_height),
                    std::to_string(pos_x), std::to_string(pos_y), image_file});
    this->graph.tail->connect(0, out_node, 0);

    this->graph.outputs.push_back(out_node);
  }

  void addCalculate(std::string type_str,
                    std::function<double(const std::vector<double> &)> _fn);

  void optimize();

  void simulate_reset();
  std::pair<double, std::string> simulate_once(bool accurate_result = false);
  std::pair<double, std::string> simulate_method(int rand_times = -1);
  std::pair<double, std::string> simulate();
  std::pair<double, std::string> simulate_conf(std::string RNS_config);

  std::pair<double, std::string>
  simulate(std::vector<std::vector<std::vector<Node *>>> possible_cliques);

  std::string simulate_LFSR_Sharing() {
    srand(this->conf.seed);

    optimize();

    if (this->graph.all_nodes.empty())
      this->graph.generate_all_nodes();

    for (const auto node : this->graph.all_nodes) {
      node->clear();
    }

    for (size_t i = 0; i < this->graph.inputs.size() - 1; i++) {
      assert(this->graph.inputs[i]->get_next_node() == nullptr);
      this->graph.inputs[i]->set_next_node(this->graph.inputs[i + 1]);
    }

    assert(this->graph.inputs[this->graph.inputs.size() - 1]->get_next_node() ==
           nullptr);

    switch (conf.simType) {
    case rand_sim:
      this->graph.inputs[0]->init({std::to_string(conf.rand_sim_times)});
      break;
    case user_def_sim:
      this->graph.inputs[0]->init({});
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

    // std::vector<std::vector<double>> MAEs(conf.rand_clique_partition_times);
    std::vector<double> MAEs;
    std::vector<std::string> confs;
    // std::unordered_map<CalculateNode *, double> sums;
    // for (const auto &cal : this->graph.calculates) {
    //   sums[cal] = 0;
    // }

    double ret_value = 0;   //= simulate_method();

    std::vector<double> min_MAEs(conf.rand_clique_partition_times);
    std::vector<std::string> min_confs(conf.rand_clique_partition_times);

    for (int i = 0; i < conf.rand_clique_partition_times; ++i) {
      if (!conf.cliquePartitionList.empty()) {
        setCILFSRMixedDomain(this->conf.cliquePartitionList[i]);
        std::cout << i << "th simulation with following clique partitioning"
                  << std::endl;
        std::stringstream clique_partition_buffer;
        for (auto clique : this->conf.cliquePartitionList[i]) {
          for (auto node_index : clique) {
            clique_partition_buffer << node_index << " ";
          }
          clique_partition_buffer << std::endl;
        }
        min_confs[i] = clique_partition_buffer.str();
        std::cout << min_confs[i];
      }
      for (int j = 0; j < conf.rand_times / conf.rand_clique_partition_times;
           ++j) {
        for (auto &sng : this->graph.SNGs) {
          if (sng->master.empty()) {
            sng->randomize();
          }
        }
        for (auto &sng : this->graph.SNGs) {
          if (!sng->master.empty()) {
            sng->copyConf(nullptr);
          }
        }
        std::pair<double, std::string> ret = simulate_once();

        MAEs.emplace_back(ret.first);
        std::cout << MAEs.size() << " "
                  << *std::min_element(MAEs.begin(), MAEs.end()) << std::endl;

        confs.push_back(ret.second);
      }

      for (const auto &cal : this->graph.calculates) {
        std::cout << cal->get_result_str() << std::endl;
        min_confs[i].append(cal->get_result_str());
      }
      min_MAEs[i] = this->graph.calculates[0]->get_value();
      this->clear_graph();
    }

    // get min clique partitioning
    if (!this->conf.cliquePartitionList.empty()) {
      int min_index =
          std::min_element(min_MAEs.begin(), min_MAEs.end()) - min_MAEs.begin();
      std::cout << "Clique partition and configuration with the least MAE"
                << std::endl;
      std::cout << min_confs[min_index] << std::endl;
    }
    auto time_end = std::chrono::high_resolution_clock::now();

    auto time =
        std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start);

    std::cout << std::endl;
    std::cout << "Time cost: " << time.count() << " seconds." << std::endl;

    std::cout << "------------------------------ simulation end   "
                 "------------------------------"
              << std::endl;

    std::cout << "confs[0]: " << std::endl;
    std::cout << confs[0] << std::endl;
    return confs[0];
  }

  void simulate_CI(std::vector<int> pattern, int index, int value) {
    std::cout << "test pattern" << std::endl;
    for (auto &i : pattern) {
      std::cout << i << " ";
    }
    std::cout << std::endl;

    std::cout << "real test pattern" << std::endl;
    int i_acc = 0;
    for (int i = 0; i < this->graph.inputs.size(); i++) {
      for (auto &out : this->graph.inputs[i]->outputs) {
        auto temp = dynamic_cast<SNGNode *>(out);
        assert(temp != nullptr);

        std::cout << (index == i_acc ? value : pattern[i_acc]) << " ";
        temp->run_CI(index == i_acc ? value : pattern[i_acc]);
        i_acc++;
      }
    }
    std::cout << std::endl;
  }

  void simulate_WCI(std::vector<int> pattern) {
    for (auto &i : pattern) {
      std::cout << i << " ";
    }
    std::cout << std::endl;

    int i_acc = 0;
    for (auto &input : this->graph.inputs) {
      for (auto &out : input->outputs) {
        auto temp = dynamic_cast<SNGNode *>(out);
        assert(temp != nullptr);
        temp->run_CI(pattern[i_acc++]);
      }
    }
  }
  void simulate_WCI_MixedDomain(std::vector<int> pattern,
                                std::vector<int> inputs) {
    int i_acc = 0;
    for (auto &input : inputs) {
      auto temp = dynamic_cast<SNGNode *>(this->graph.all_nodes[input]);
      assert(temp != nullptr);
      temp->run_CI(pattern[i_acc++]);
    }
  }

  void simulate_CI_SC(std::vector<int> pattern, int index, int value,
                      std::vector<int> inputs) {

    int i_acc = 0;
    for (int input : inputs) {
      auto temp = dynamic_cast<SNGNode *>(this->graph.all_nodes[input]);
      assert(temp != nullptr);
      temp->run_CI(index == i_acc ? value : pattern[i_acc]);
      i_acc++;
    }
    //		std::cout<<std::endl;
  }

  // exclude node without Verilog definition
  std::unordered_map<Node *, int>
  exclude_non_verilog_node(std::unordered_map<Node *, int> node_index_map) {
    // std::cout << "==== Exclude non verilog node func not used. ===="
    //           << std::endl;
    return node_index_map;
  }

  void search(double target_error);

  bool find(Var &circuit, double &error, const double &target_error);

  Var replace(Node *node);

  Node *SC_equivalence(Node *node);

  std::vector<Node *> all_BC_nodes();

  void genVerilog();

  void genVerilogTestBench(std::vector<int> input_index, std::string file_name);

  void clear_graph();

  std::vector<std::pair<int, int>> findCIPair() {
    std::vector<std::pair<int, int>> res(0);
    int input_size = 0;   // one input may be connected to multiple LFSR
    for (const auto &in : this->graph.inputs) {
      for (const auto &out : in->outputs) {
        assert(dynamic_cast<LFSRNode *>(out) != nullptr);
        input_size++;
      }
    }

    std::vector<std::vector<int>> detect_sets(input_size);
    for (int i = 0; i < input_size; i++) {
      for (int index_pattern = 0; index_pattern < int(pow(2, input_size));
           index_pattern++) {
        // generate test pattern
        bitstream_list test_pattern = num_to_arr(input_size, index_pattern);

        // correct
        std::cout << "correct: " << std::endl;
        simulate_CI(test_pattern, -1, 0);

        int correct_out = this->graph.tail->data[0];
        std::cout << "res: " << correct_out << std::endl;
        // detect single stuck-at model for i

        // fault
        std::cout << "fault " << i << std::endl;
        simulate_CI(test_pattern, i, !test_pattern[i]);
        int fault_out_i = this->graph.tail->data[0];
        std::cout << "res: " << fault_out_i << std::endl;
        if (correct_out != fault_out_i)   // detect
        {
          detect_sets[i].emplace_back(index_pattern);
        }
      }
    }
    for (int i = 0; i < input_size; i++) {
      for (int j = i + 1; j < input_size; j++) {
        std::vector<int> temp = {};
        std::set_intersection(detect_sets[i].begin(), detect_sets[i].end(),
                              detect_sets[j].begin(), detect_sets[j].end(),
                              std::back_inserter(temp));
        // i,j are CI pair
        if (temp.empty()) {
          res.emplace_back(std::pair<int, int>(i, j));
        }
      }
    }

    std::cout << "Total " << res.size() << " CI pairs: " << std::endl;
    for (auto &i : res) {
      std::cout << i.first << " " << i.second << std::endl;
    }

    clear_graph();
    return res;
  }

  bool testCofactorCI(WCI_set wci_set, std::vector<int> cofactor) {
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
        std::cout << "correct: " << std::endl;
        simulate_WCI(pattern);

        int correct_out = this->graph.tail->data[0];
        std::cout << "res: " << correct_out << std::endl;

        // detect single stuck-at model for i

        pattern[wci_set[i]] = !pattern[wci_set[i]];
        // fault
        std::cout << "fault " << wci_set[i] << std::endl;
        simulate_WCI(pattern);
        int fault_out_i = this->graph.tail->data[0];
        std::cout << "res: " << fault_out_i << std::endl;

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
          std::cout << "find CI pair " << wci_set[i] << " " << wci_set[j]
                    << std::endl;
          std::cout << "Cofactor ";
          for (auto &k : cofactor) {
            std::cout << k << " ";
          }
          std::cout << std::endl;
          return true;
        }
      }
    }
    clear_graph();
    return false;
  }
  bool testCofactorCIMixdeDomain(WCI_set wci_set, int tail,
                                 std::vector<int> inputs,
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
        simulate_WCI_MixedDomain(pattern, inputs);

        int correct_out = this->graph.all_nodes[tail]->data[0];

        // detect single stuck-at model for i
        pattern[wci_set[i]] = !pattern[wci_set[i]];
        // fault
        simulate_WCI_MixedDomain(pattern, inputs);
        int fault_out_i = this->graph.all_nodes[tail]->data[0];

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
          //						std::cout << "find CI pair " <<
          //wci_set[i]
          //<< " " << wci_set[j] << std::endl; 						std::cout
          //<< "Cofactor "; 						for
          //(auto &k:cofactor){ 							std::cout << k << "
          //";
          //						}
          //						std::cout<<std::endl;
          return true;
        }
      }
    }
    clear_graph();
    return false;
  }
  std::vector<WCI_set>
  findWCISet(const std::vector<std::pair<int, int>> &ci_set = {},
             std::vector<int> input_indexs = {}) {
    // Identical to the paper, we only find weakly CI set of size 3
    std::vector<WCI_set> res;
    int input_size = 0;   // one input may be connected to multiple LFSR
    for (const auto &in : this->graph.inputs) {
      for (const auto &out : in->outputs) {
        assert(dynamic_cast<LFSRNode *>(out) != nullptr);
        input_size++;
      }
    }
    if (input_indexs.empty()) {
      for (int i = 0; i < input_size; i++) {
        input_indexs.emplace_back(i);
      }
    }
    // currently only consider WCI_set in graph with no less than 3 inputs
    assert(input_indexs.size() >= 3);
    // for all possible set of size 3
    for (int i = 0; i < input_indexs.size(); i++) {
      for (int j = i + 1; j < input_indexs.size(); j++) {
        for (int k = j + 1; k < input_indexs.size(); k++) {
          bool contain_ci_pair = false;
          for (auto ci_pair : ci_set) {
            if (i == ci_pair.first || j == ci_pair.first ||
                k == ci_pair.first || i == ci_pair.second ||
                j == ci_pair.second || k == ci_pair.second) {
              contain_ci_pair = true;
            }
          }
          if (contain_ci_pair) {
            std::cout << "WCI set contain CI pair" << input_indexs[i]
                      << input_indexs[j] << input_indexs[k] << std::endl;
            continue;
          }
          std::cout << "possible WCI set" << input_indexs[i] << input_indexs[j]
                    << input_indexs[k] << std::endl;
          std::vector<int> cube(input_indexs.size(), 0);
          cube[input_indexs[i]] = -1;
          cube[input_indexs[j]] = -1;
          cube[input_indexs[k]] = -1;

          bool is_weakly = true;
          // for each cofactor
          for (int cofactor_value = 0;
               cofactor_value < int(pow(2, input_indexs.size() - 3));
               cofactor_value++) {
            bitstream_list cofactor =
                num_to_arr(input_indexs.size() - 3, cofactor_value);
            int temp = 0;
            for (int cube_index = 0; cube_index < input_indexs.size();
                 cube_index++) {
              if (cube[cube_index] == -1)
                temp--;
              else
                cube[cube_index] = cofactor[cube_index + temp];
            }
            assert(temp == -3);
            if (!testCofactorCI(
                    {input_indexs[i], input_indexs[j], input_indexs[k]},
                    cube)) {
              is_weakly = false;
              break;
            }
          }
          if (is_weakly) {
            res.push_back({input_indexs[i], input_indexs[j], input_indexs[k]});
          }
        }
      }
    }
    std::cout << "WCI set: " << std::endl;
    for (auto &wci_set : res)
      std::cout << wci_set[0] << " " << wci_set[1] << " " << wci_set[2]
                << std::endl;
    return res;
  }

  std::vector<WCI_set> findWCISetMixDomain(int tail,
                                           std::vector<int> input_indexs) {
    // Identical to the paper, we only find weakly CI set of size 3
    std::vector<WCI_set> res;

    // currently only consider WCI_set in graph with no less than 3 inputs
    if (input_indexs.size() < 3)
      return res;
    // for all possible set of size 3
    for (int i = 0; i < input_indexs.size(); i++) {
      for (int j = i + 1; j < input_indexs.size(); j++) {
        for (int k = j + 1; k < input_indexs.size(); k++) {
          bool contain_ci_pair = false;

          //					std::cout<< "possible WCI
          // set"<<input_indexs[i]<<input_indexs[j]<<input_indexs[k]<<std::endl;
          std::vector<int> cube(input_indexs.size(), 0);
          cube[i] = -1;
          cube[j] = -1;
          cube[k] = -1;

          bool is_weakly = true;
          // for each cofactor
          for (int cofactor_value = 0;
               cofactor_value < int(pow(2, input_indexs.size() - 3));
               cofactor_value++) {
            bitstream_list cofactor =
                num_to_arr(input_indexs.size() - 3, cofactor_value);
            int temp = 0;
            for (int cube_index = 0; cube_index < input_indexs.size();
                 cube_index++) {
              if (cube[cube_index] == -1)
                temp--;
              else
                cube[cube_index] = cofactor[cube_index + temp];
            }
            assert(temp == -3);
            if (!testCofactorCIMixdeDomain({i, j, k}, tail, input_indexs,
                                           cube)) {
              is_weakly = false;
              break;
            }
          }
          if (is_weakly) {
            res.push_back({input_indexs[i], input_indexs[j], input_indexs[k]});
          }
        }
      }
    }
    return res;
  }

  std::map<int, std::set<int>> extractVertex(std::vector<CI_pair> edge_set) {
    std::map<int, std::set<int>> v_e_map;
    for (auto &i : edge_set) {
      v_e_map[i.first].insert(i.second);
      v_e_map[i.second].insert(i.first);
    }
    return v_e_map;
  }

  std::map<Node *, std::set<Node *>>
  get_vertex(std::vector<std::pair<Node *, Node *>> edge_set);

  std::vector<std::vector<Node *>>
  min_clique_cover(std::vector<std::pair<Node *, Node *>> edge_set);

  CliqueList minCliqueCover(std::vector<CI_pair> edge_set,
                            std::vector<int> input_indexs) {
    CliqueList clique_list = {};
    std::map<int, std::set<int>> v_e_map = extractVertex(edge_set);

    std::vector<std::pair<int, int>> v_degree(input_indexs.size());

    for (int i = 0; i < v_degree.size(); i++) {
      v_degree[i] =
          std::pair<int, int>(input_indexs[i], v_e_map[input_indexs[i]].size());
    }
    // sort based on degree
    std::sort(v_degree.begin(), v_degree.end(),
              [](std::pair<int, int> a, std::pair<int, int> b) {
                return a.second > b.second;
              });
    for (int &input_index : input_indexs) {
      // try to put a new vertice into available clique
      std::vector<int> available_cliqiues = {};
      for (int clique_index = 0; clique_index < clique_list.size();
           clique_index++) {
        bool fit_in = true;
        // a vertice can be added to a clique if it has edges with all vertices
        // in the clique
        for (auto &vert : clique_list[clique_index]) {
          if (std::find(v_e_map[input_index].begin(),
                        v_e_map[input_index].end(),
                        vert) == v_e_map[input_index].end()) {
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
        clique_list.push_back({input_index});
      } else {
        clique_list[available_cliqiues[rand() % available_cliqiues.size()]]
            .emplace_back(input_index);
      }
    }
    return clique_list;
  }

  void setCILFSR(const std::vector<std::pair<int, int>> &CI_set,
                 std::vector<int> input_indexs = {}) {
    // CI
    std::vector<int> input_lfsr_num(
        0);   // one input may be connected to multiple LFSR
    int input_size = 0;
    for (const auto &in : this->graph.inputs) {
      for (const auto &out : in->outputs) {
        assert(dynamic_cast<LFSRNode *>(out) != nullptr);
        input_size++;
      }
      input_lfsr_num.emplace_back(input_size);
    }
    if (input_indexs.empty()) {
      for (int i = 0; i < input_size; i++) {
        input_indexs.emplace_back(i);
      }
    }

    CliqueList cliqueList = minCliqueCover(CI_set, input_indexs);

    for (auto clique : cliqueList) {
      if (clique.size() > 1) {
        int first_index = fall_in_interval(clique[0], input_lfsr_num);
        auto first = dynamic_cast<SNGNode *>(
            this->graph.inputs[first_index]
                ->outputs[first_index == 0
                              ? clique[0]
                              : clique[0] - input_lfsr_num[first_index - 1]]);
        assert(first != nullptr);
        assert(first->master.empty());
        for (int remain = 1; remain < clique.size(); remain++) {
          int second_index = fall_in_interval(clique[remain], input_lfsr_num);
          auto second = dynamic_cast<SNGNode *>(
              this->graph.inputs[second_index]
                  ->outputs[second_index == 0
                                ? clique[remain]
                                : clique[remain] -
                                      input_lfsr_num[second_index - 1]]);
          assert(second != nullptr);
          assert(first->master.empty());
          second->master = {first};
        }
      }
    }
  }

  std::vector<std::vector<std::vector<Node *>>>
  find_clique_partition(std::vector<std::pair<Node *, Node *>> CI_set);

  void findCliquePartition(std::vector<CI_pair> CI_set) {
    if (this->graph.all_nodes.empty())
      this->graph.generate_all_nodes();

    std::vector<int> sng_indexs;
    for (auto node : this->graph.all_nodes) {
      if (dynamic_cast<SNGNode *>(node) != nullptr) {
        sng_indexs.emplace_back(this->graph.node_id_dict[node]);
      }
    }

    srand(time(nullptr));
    for (int i = 0; i < this->conf.rand_clique_partition_times; i++) {
      CliqueList cliqueList = minCliqueCover(CI_set, sng_indexs);
      this->conf.cliquePartitionList.emplace_back(cliqueList);
    }
  }

  void set_CI_in_BC_SC(const std::vector<std::vector<Node *>> &clique_list);

  void setCILFSRMixedDomain(const CliqueList &cliqueList) {
    if (this->graph.all_nodes.empty())
      this->graph.generate_all_nodes();
    std::unordered_map<Node *, int> node_id_dict;
    for (size_t i = 0; i < this->graph.all_nodes.size(); i++) {
      node_id_dict.emplace(this->graph.all_nodes[i], i);
    }

    std::vector<int> sng_indexs;
    for (auto node : this->graph.all_nodes) {
      if (dynamic_cast<SNGNode *>(node) != nullptr) {
        sng_indexs.emplace_back(node_id_dict[node]);
      }
    }

    for (auto clique : cliqueList) {
      if (clique.size() > 1) {
        auto first = dynamic_cast<SNGNode *>(this->graph.all_nodes[clique[0]]);
        first->master.clear();
        for (int remain = 1; remain < clique.size(); remain++) {
          auto second =
              dynamic_cast<SNGNode *>(this->graph.all_nodes[clique[remain]]);
          assert(second != nullptr);
          second->master = {first};
        }
      }
    }
    this->clear_graph();
  }

  void setWCILFSR(const std::vector<WCI_set> &wci_sets,
                  std::vector<int> input_indexs = {}) {
    // WCI
    std::vector<int> input_lfsr_num(
        0);   // one input may be connected to multiple LFSR
    int input_size = 0;
    for (const auto &in : this->graph.inputs) {
      for (const auto &out : in->outputs) {
        assert(dynamic_cast<LFSRNode *>(out) != nullptr);
        input_size++;
      }
      input_lfsr_num.emplace_back(input_size);
    }
    if (input_indexs.empty()) {
      for (int i = 0; i < input_size; i++) {
        input_indexs.emplace_back(i);
      }
    }

    std::vector<WCI_set> used_wci_sets = {};

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
        int first_index = fall_in_interval(wci_set[0], input_lfsr_num);
        auto first = dynamic_cast<SNGNode *>(
            this->graph.inputs[first_index]
                ->outputs[first_index == 0
                              ? wci_set[0]
                              : wci_set[0] - input_lfsr_num[first_index - 1]]);
        assert(first != nullptr);
        int second_index = fall_in_interval(wci_set[1], input_lfsr_num);
        auto second = dynamic_cast<SNGNode *>(
            this->graph.inputs[second_index]
                ->outputs[second_index == 0
                              ? wci_set[1]
                              : wci_set[1] - input_lfsr_num[second_index - 1]]);
        assert(second != nullptr);
        int third_index = fall_in_interval(wci_set[2], input_lfsr_num);
        auto third = dynamic_cast<SNGNode *>(
            this->graph.inputs[third_index]
                ->outputs[third_index == 0
                              ? wci_set[2]
                              : wci_set[2] - input_lfsr_num[third_index - 1]]);
        assert(third != nullptr);
        assert(third->master.empty());
        third->master = {first, second};
      }
    }
    std::cout << "The following WCI set is used to reduced area" << std::endl;
    for (auto wci_set : used_wci_sets) {
      std::cout << wci_set[0] << " " << wci_set[1] << " " << wci_set[2]
                << std::endl;
    }
  }

  void set_WCI_in_BC_SC(const std::vector<std::vector<Node *>> &wci_sets);

  void setWCILFSRMixdeDomain(const std::vector<WCI_set> &wci_sets) {
    // WCI

    std::vector<WCI_set> used_wci_sets = {};

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
        auto first = dynamic_cast<SNGNode *>(this->graph.all_nodes[wci_set[0]]);
        assert(first != nullptr);
        auto second =
            dynamic_cast<SNGNode *>(this->graph.all_nodes[wci_set[1]]);
        assert(second != nullptr);
        auto third = dynamic_cast<SNGNode *>(this->graph.all_nodes[wci_set[2]]);
        assert(third != nullptr);
        assert(third->master.empty());
        third->master = {first, second};
      }
    }
    std::cout << "The following WCI set is used to reduced area" << std::endl;
    for (auto wci_set : used_wci_sets) {
      std::cout << wci_set[0] << " " << wci_set[1] << " " << wci_set[2]
                << std::endl;
    }
  }

  std::vector<Node *> domain_tree_search_2(Node *root, VAR_TYPE search_type);

  std::vector<int> domain_tree_search(Node *root, VAR_TYPE type_to_search) {
    std::vector<int> res;
    std::stack<Node *> stack_to_search;
    stack_to_search.emplace(root);

    while (!stack_to_search.empty()) {
      auto *current_root = stack_to_search.top();
      stack_to_search.pop();
      switch (type_to_search) {
      case BC:
        if (dynamic_cast<APCNode *>(current_root) != nullptr ||
            dynamic_cast<CounterNode *>(current_root) != nullptr ||
            dynamic_cast<SCTanhNode *>(current_root) != nullptr) {
          for (auto *child_node : current_root->inputs) {
            if (std::find(res.begin(), res.end(),
                          this->graph.node_id_dict[child_node]) == res.end())
              // consider reconvergent circuits. one node can be added multiple
              // times
              res.emplace_back(this->graph.node_id_dict[child_node]);
          }
        } else {
          for (auto *child_node : current_root->inputs) {
            stack_to_search.emplace(child_node);
          }
        }

        break;
      case SC:
        if (dynamic_cast<SCTanhNode *>(current_root) != nullptr) {
          // TODO
          // now SC domain search terminate when encountering SCTanh
          res.emplace_back(this->graph.node_id_dict[current_root]);
        }

        if (dynamic_cast<SNGNode *>(current_root) != nullptr) {
          if (std::find(res.begin(), res.end(),
                        this->graph.node_id_dict[current_root]) == res.end())
            // consider reconvergent circuits. one node can be added multiple
            // times
            res.emplace_back(this->graph.node_id_dict[current_root]);
          // the tail of BC domain can be SNGs, since we do not need to manually
          // search, that is Okay
        } else {
          for (auto *child_node : current_root->inputs) {
            stack_to_search.emplace(child_node);
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
  find_CI_in_BC(Node *tail, std::vector<Node *> inputs);

  std::vector<CI_pair> findCIPairBCDomain(int tail, std::vector<int> inputs) {
    std::vector<CI_pair> res(0);
    for (int i = 0; i < inputs.size(); i++) {
      for (int j = i + 1; j < inputs.size(); j++) {
        res.emplace_back(std::pair<int, int>(inputs[i], inputs[j]));
      }
    }

    //		std::cout<< "BC Domain Total "<< res.size()<<" CI pairs:
    //" <<
    // std::endl; 		for (auto &i:res){
    // std::cout<< i.first << " "<< i.second
    // << std::endl;
    //		}

    return res;
  }

  std::vector<std::pair<Node *, Node *>>
  find_CI_in_SC(Node *tail, std::vector<Node *> inputs);

  std::vector<CI_pair> findCIPairSCDomain(int tail, std::vector<int> inputs) {
    std::vector<CI_pair> res(0);
    std::vector<std::vector<int>> detect_sets(inputs.size());
    for (int i = 0; i < inputs.size(); i++) {
      for (int index_pattern = 0; index_pattern < int(pow(2, inputs.size()));
           index_pattern++) {
        // generate test pattern
        bitstream_list test_pattern = num_to_arr(inputs.size(), index_pattern);

        //				std::cout<<"test pattern ";
        //        for (auto ii:test_pattern){
        //          std::cout<< ii<< " ";
        //        }
        //        std::cout<<std::endl;
        // correct
        simulate_CI_SC(test_pattern, -1, 0, inputs);

        int correct_out = this->graph.all_nodes[tail]->data[0];
        // detect single stuck-at model for i

        // fault
        simulate_CI_SC(test_pattern, i, !test_pattern[i], inputs);
        int fault_out_i = this->graph.all_nodes[tail]->data[0];
        // std::cout<<"res: "<< fault_out_i<<std::endl;
        if (correct_out != fault_out_i)   // detect
        {
          detect_sets[i].emplace_back(index_pattern);
        }
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
          res.emplace_back(std::pair<int, int>(inputs[i], inputs[j]));
        }
      }
    }

    //		std::cout<< "SC Domain Total "<< res.size()<<" CI pairs:
    //" <<
    // std::endl; 		for (auto &i:res){
    // std::cout<< i.first << " "<< i.second
    // << std::endl;
    //		}

    clear_graph();

    return res;
  }

  void updateCIPair(std::vector<CI_pair> &CI_pairs,
                    std::vector<CI_pair> new_CI_pairs, int tail,
                    const std::vector<int> &inputs,
                    std::map<int, std::vector<int>> lfsr_sibling_dict) {
    // SourceNode -> LFSR will result in an empty BC domain tree
    if (inputs.empty())
      return;
    // add new CI pairs
    CI_pairs.insert(CI_pairs.end(), new_CI_pairs.begin(), new_CI_pairs.end());
    // update existing relationship
    std::vector<CI_pair> temp_CI_pairs;

    for (auto &CI_pair : CI_pairs) {
      // std::cout<<"pair updating" <<  CI_pair.first
      // <<CI_pair.second<<std::endl;
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

    if (dynamic_cast<SNGNode *>(this->graph.all_nodes[tail]) == nullptr)
      // keep it if it's a LFSR
      std::erase_if(CI_pairs, [tail](CI_pair ci_pair) {
        return tail == ci_pair.first || tail == ci_pair.second;
      });

    if (lfsr_sibling_dict.contains(tail)) {
      // tail is LFSR -> current is BC domain
      for (auto &lfsr : lfsr_sibling_dict[tail]) {
        for (auto &input : inputs) {
          // add new CI relationship if it doesn't exist
          if ((std::find(CI_pairs.begin(), CI_pairs.end(),
                         CI_pair{input, lfsr}) == CI_pairs.end()) &&
              (std::find(CI_pairs.begin(), CI_pairs.end(),
                         CI_pair{lfsr, input}) == CI_pairs.end())) {
            CI_pairs.emplace_back(CI_pair{input, lfsr});
          }
        }
      }
    }
  }

  void CIDetect(int rand_clique_partition_times_ = 10) {
    // find and set CI/WCI LFSR
    this->conf.rand_clique_partition_times = rand_clique_partition_times_;
    std::vector<CI_pair> CI_pairs;
    std::vector<WCI_set> WCI_sets;
    findCIWCIMixedDomain(CI_pairs, WCI_sets);
    findCliquePartition(CI_pairs);
    setWCILFSRMixdeDomain(WCI_sets);
  }

  std::vector<std::vector<std::vector<Node *>>>
  detect_CI(int rand_clique_partition_times_ = 10) {
    // find and set CI/WCI LFSR
    this->conf.rand_clique_partition_times = rand_clique_partition_times_;
    std::vector<std::pair<Node *, Node *>> CI_pairs;
    std::vector<std::vector<Node *>> WCI_sets;
    find_CI_WCI_in_BC_SC(CI_pairs, WCI_sets);

    std::vector<std::vector<std::vector<Node *>>> possible_cliques;
    possible_cliques = find_clique_partition(CI_pairs);

    set_WCI_in_BC_SC(WCI_sets);
    return possible_cliques;
  }

  void find_CI_WCI_in_BC_SC(std::vector<std::pair<Node *, Node *>> &CI_pairs,
                            std::vector<std::vector<Node *>> &WCI_sets);

  void findCIWCIMixedDomain(std::vector<CI_pair> &CI_pairs,
                            std::vector<WCI_set> &WCI_sets) {
    // Must be used before addCalculate
    // construct node to id dict
    if (this->graph.all_nodes.empty())
      this->graph.generate_all_nodes();

    std::vector<int> sng_indexs;
    for (auto node : this->graph.SNGs) {
      sng_indexs.emplace_back(this->graph.node_id_dict[node]);
    }

    std::stack<Node *> stack_to_search;
    std::stack<VAR_TYPE> type_stack_to_search;
    std::map<int, std::vector<int>> lfsr_sibling_dict;

    stack_to_search.emplace(this->graph.tail);
    type_stack_to_search.emplace(this->type);
    while (!stack_to_search.empty()) {
      auto *root_node = stack_to_search.top();
      stack_to_search.pop();
      auto type_to_search = type_stack_to_search.top();
      type_stack_to_search.pop();
      bool contain_SCTanh = false;
      if (dynamic_cast<SourceNode *>(root_node) != nullptr)
        continue;
      std::vector<int> children;
      std::vector<CI_pair> CI_result;   // CI pairs for Partial Domain Tree
      switch (type_to_search) {
      case BC: {
        // std::cout<<"BC"<<std::endl;
        children = domain_tree_search(root_node, BC);

        for (auto child : children) {
          stack_to_search.emplace(this->graph.all_nodes[child]);
          type_stack_to_search.emplace(SC);
        }

        CI_result =
            findCIPairBCDomain(this->graph.node_id_dict[root_node], children);

        break;
      }

      case SC: {
        // std::cout<<"SC"<<std::endl;
        children = domain_tree_search(root_node, SC);

        for (auto child : children) {   // sng, SCtanh
          assert(this->graph.all_nodes[child]->inputs.size() == 1);
          stack_to_search.emplace(this->graph.all_nodes[child]);
          type_stack_to_search.emplace(BC);
          if (dynamic_cast<SCTanhNode *>(this->graph.all_nodes[child]) !=
              nullptr) {
            contain_SCTanh = true;
          }
          lfsr_sibling_dict[child] = children;
        }
        // children: sngs; tail: APC or Counter (Destination Node not
        // possible)
        if (contain_SCTanh)
          continue;

        CI_result =
            findCIPairSCDomain(this->graph.node_id_dict[root_node], children);
        WCI_sets =
            findWCISetMixDomain(this->graph.node_id_dict[root_node], children);
        break;
      }
      default:
        break;
      }

      updateCIPair(CI_pairs, CI_result, this->graph.node_id_dict[root_node],
                   children, lfsr_sibling_dict);
    }

    // exclude CI pair with the same inputs
    std::erase_if(CI_pairs, [](CI_pair ci_pair) {
      return ci_pair.first == ci_pair.second;
    });

    // exclude CI pair from WCI set
    std::erase_if(WCI_sets, [CI_pairs](WCI_set wci_set) {
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

  void expr();

  void config_by_str(const std::string &str);

  int simulate_CI_in_SC(std::vector<int> pattern, int index, int value,
                        std::vector<Node *> inputs, Node *tail);

  std::vector<std::vector<Node *>>
  find_WCI_in_BC_SC(Node *tail, std::vector<Node *> inputs);

  bool test_cofactor_CI_in_BC_SC(WCI_set wci_set, Node *tail,
                                 std::vector<Node *> inputs,
                                 std::vector<int> cofactor);

  int simulate_WCI_in_BC_SC(std::vector<int> pattern,
                            std::vector<Node *> inputs, Node *tail);
  void update_CI_pair(std::vector<std::pair<Node *, Node *>> &CI_pairs,
                      std::vector<std::pair<Node *, Node *>> new_CI_pairs,
                      Node *tail, const std::vector<Node *> &inputs,
                      std::map<Node *, std::vector<Node *>> lfsr_sibling_map);
};

}   // namespace SC_sim

#endif
