#ifndef SC_CORE_H
#define SC_CORE_H

#include "Headers.h"
#include "Nodes/Node.h"
#include "Nodes/NodeFactory.h"
#include "Nodes/OptimizeNode.h"
#include "Utils/Utils.h"
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * Not very useful, because for some circuit, the current bit output relies on
 * the previous stages (e.g. cos(x) circuit).
 */
class SC_core {
private:
  int N;
  int LEN;
  std::function<double(std::vector<int>)> real_fn;
  std::function<int(std::vector<int>)> simulate_fn;

public:
  SC_core(const int &N, const std::function<double(std::vector<int>)> &_real_fn,
          const std::function<int(std::vector<int>)> &_simulate_fn) {
    this->N = N;
    LEN = std::round(std::pow(2, N));
    real_fn = _real_fn;
    simulate_fn = _simulate_fn;
  }

  int simulate(std::vector<int> inputs);

  bitstream_list simulate_list(std::vector<bitstream_list> input_lists);

  double simulate_list_value(std::vector<bitstream_list> input_lists);

  /**
   *
   * @param input_lists
   * @param input_values used for calculation of real value
   * @return MAE
   */
  double simulate_MAE(std::vector<bitstream_list> input_lists,
                      std::vector<int> input_values);
};

// SC core design
class SC {
protected:
  std::vector<SourceNode *> inputs;
  std::vector<DestNode *> outputs;
  std::vector<CalculateNode *> calculates;

  std::vector<SNGNode *> SNGs;

  std::unordered_set<Node *> all_nodes;

  std::function<double(const std::vector<double> &)> fn;

  Config conf;

public:
  SC(){};
  SC(std::string file_name){};

  // ~SC() {
  //   for (auto const &node : all_nodes) {
  //     delete (node);
  //   }
  // }

  void set_fn(std::function<double(const std::vector<double> &)> _fn) {
    fn = _fn;
  }

  void set_conf(const auto &conf_new) { conf = conf_new; }

  void reset() {
    inputs[0]->clear_ready();
    for (const auto &node : all_nodes) {
      node->reset();
    }
  }

  void finish() {
    for (const auto &node : all_nodes) {
      node->finish();
    }
  }

  virtual void init() = 0;

  void generate_all_nodes() {
    std::vector<Node *> queue;
    for (const auto &in : inputs)
      queue.push_back(in);

    // for (const auto &input : inputs) {
    //   for (const auto &input_out : input->outputs) {
    //     if (SNGNode *temp = dynamic_cast<SNGNode *>(input_out)) {
    //       SNGs.push_back(temp);
    //     }
    //   }
    // }

    while (!queue.empty()) {
      Node *now = queue.front();
      queue.erase(queue.begin());
      for (auto const &out : now->outputs)
        queue.push_back(out);

      if (all_nodes.contains(now))
        continue;
      all_nodes.insert(now);
    }

    // for (const auto &node : all_nodes) {
    //   if (SNGNode *temp = dynamic_cast<SNGNode *>(node)) {
    //     SNGs.push_back(temp);
    //   }
    // }
  }

  void BFS(FunctionNode *start, std::vector<FunctionNode *> &cluster_starts,
           std::vector<FunctionNode *> &cluster_ends,
           std::vector<FunctionNode *> cluster_nodes) {
    std::vector<FunctionNode *> queue;
    std::unordered_set<Node *> visited;

    queue.push_back(start);
    while (!queue.empty()) {
      FunctionNode *now = queue.at(0);
      queue.erase(queue.begin());
      if (visited.contains(now))
        continue;
      visited.insert(now);
      cluster_nodes.push_back(now);

      bool is_start = false;
      for (const auto input : now->inputs) {
        if (dynamic_cast<FunctionNode *>(input)) {
          if (!visited.contains(input))
            queue.push_back(dynamic_cast<FunctionNode *>(input));
        } else {
          is_start = true;
        }
      }
      if (is_start) {
        cluster_starts.push_back(now);
      }

      bool is_end = true;
      for (const auto output : now->outputs) {
        if (dynamic_cast<FunctionNode *>(output)) {
          if (!visited.contains(output))
            queue.push_back(dynamic_cast<FunctionNode *>(output));
          is_end = false;
        }
      }
      if (is_end) {
        cluster_ends.push_back(now);
      }
    }
  }

  void optimize() {
    if (conf.optimize & RUNNING_MINIMAL) {
      for (const auto &input : inputs) {
        input->minNode = calculates[0];
      }
    }
    if (conf.optimize & LOOKUPTABLE) {
      // Lookup table optimization
      // Use BFS to find a cluster of ANDNode.
      // std::vector<Node *> and_nodes;
      std::vector<FunctionNode *> cluster_starts;
      std::vector<FunctionNode *> cluster_ends;
      std::vector<FunctionNode *> cluster_nodes;
      for (const auto node : all_nodes) {
        if (dynamic_cast<FunctionNode *>(node)) {
          BFS(dynamic_cast<FunctionNode *>(node), cluster_starts, cluster_ends,
              cluster_nodes);
          break;
        }
      }
      // Replace the cluster with a LookupTableNode
      int input_num = 0;
      int output_num = 0;
      for (const auto start : cluster_starts) {
        for (const auto in : start->inputs) {
          if (!dynamic_cast<FunctionNode *>(in))
            input_num++;
        }
      }
      for (const auto end : cluster_ends) {
        output_num += end->output_num;
      }

      LookupTableNode *lookup_table =
          new LookupTableNode(input_num, output_num, conf);
      int index_input = 0;
      for (const auto start : cluster_starts) {
        for (const auto input : start->inputs) {
          if (!dynamic_cast<FunctionNode *>(input)) {
            input->set_out_node(input->find_out_node(start), lookup_table);
            lookup_table->set_in_node(index_input++, input);
          }
        }
      }
      int index_output = 0;
      for (const auto end : cluster_ends) {
        for (const auto output : end->outputs) {
          output->set_in_node(output->find_in_node(end), lookup_table);
          lookup_table->set_out_node(index_output++, output);
        }
      }
      lookup_table->add_inside_circuits(cluster_starts, cluster_ends);
    }
  }

  void simulate() {
    std::cout << "------------------------------ simulation start "
                 "------------------------------"
              << std::endl;
    std::cout << conf << std::endl;

    auto time_start = std::chrono::high_resolution_clock::now();

    // std::unordered_map<CalculateNode *, double> sums;
    // for (const auto &cal : calculates) {
    //   sums[cal] = 0;
    // }

    for (int i = 0; i < conf.rand_times; ++i) {
      // The order of sng to randomize matters in order to keep the randomness.
      for (const auto &sng : SNGs) {
        sng->randomize();
      }
      reset();
      inputs[0]->run();
      // for (const auto &cal : calculates) {
      //   sums[cal] += cal->get_value();
      // }
      finish();
    }
    // for (const auto &[node, sum] : sums) {
    //   std::cout << "Value of Node " << node->get_label() << " is "
    //             << sum / conf.rand_times << std::endl;
    // }
    for (const auto &cal : calculates) {
      std::cout << cal->get_result_str() << std::endl;
    }

    auto time_end = std::chrono::high_resolution_clock::now();

    auto time =
        std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start);

    std::cout << std::endl;
    std::cout << "Time cost: " << time.count() << " seconds." << std::endl;

    std::cout << "------------------------------ simulation end   "
                 "------------------------------"
              << std::endl;
    return;
  }
};

#endif
