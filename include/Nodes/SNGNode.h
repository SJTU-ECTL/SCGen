/**
 * @brief
 *     The definition and implementation of abstract class SNGNode and all its
 *         derived specific SNG node.
 */

#ifndef SNGNODE_H
#define SNGNODE_H

#include "Modules/LFSR.h"
#include "Modules/SNG.h"
#include "Modules/Sobol.h"
#include "Node.h"
#include "Utils/Utils.h"
#include "Utils/Utils_IO.h"
#include <cassert>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

class ConfigState {
public:
  int rand = false;
  int poly_index;
  int seed_index;
  int scram_index;

  ConfigState() { rand = true; }
};
class SNGNode : public Node {
private:
public:
  std::vector<int> master_id;
  std::vector<SNGNode *> master;
  // one: copy from this SNG; two: xor of these two SNG

  SNGNode(int _input_num, int _output_num, const Config &_conf,
          std::vector<SNGNode *> _master = {})
      : Node(_input_num, _output_num, _conf) {
    assert(_input_num == 1);
    master = std::move(_master);
  }

  virtual std::string get_config() = 0;

  virtual std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
    return "";
  }
  virtual std::string verilog_module_init_str() { return ""; }

  virtual void run_CI(int i) {
    data = std::vector<int>(1);
    clear_ready();
    data[0] = i;
    ready = true;
    notify();
  }
  virtual ConfigState randomize(ConfigState state = ConfigState()) = 0;
  virtual ConfigState copyConf(SNGNode *node) = 0;

  virtual void config_by_str(const std::string &str) = 0;

  virtual Node *copy() = 0;
};

class HypergeometricNode : public SNGNode {
private:
  int N;

  // Indicating the LFSR has been configured, otherwise gives an info about
  // using default configurations.
  bool configured = false;
  bool inserting_zero = true;
  bool leq = false;

  bitstream_list sequence;
  bitstream_list lookup_table;
  int prev = -1;

public:
  ConfigState copyConf(SNGNode *node){};
  HypergeometricNode(int _input_num, int _output_num, const Config &_conf);

  virtual Node *copy() {
    assert(0 && "Not implemented");
    return nullptr;
  }

  std::string label() { return "Hypergeometric"; };
  std::string codegen_str();

  std::string get_config();

  // Should be called before randomize()
  void configure(bool _leq, bool _inserting_zero);

  ConfigState randomize(ConfigState state);

  void reset_zero();

  void run();
  void config_by_str(const std::string &str) {}
};

class BinomialNode : public SNGNode {
private:
  int N;

  // Indicating the LFSR has been configured, otherwise gives an info about
  // using default configurations.
  bool configured = false;
  bool inserting_zero = true;
  bool leq = false;

  bitstream_list sequence;
  int prev = -1;

public:
  ConfigState copyConf(SNGNode *node) { assert(0); };
  BinomialNode(int _input_num, int _output_num, const Config &_conf);

  virtual Node *copy() {
    assert(0 && "Not implemented");
    return nullptr;
  }

  std::string label() { return "Binomial"; };
  std::string codegen_str();

  std::string get_config();

  // Should be called before randomize()
  void configure(bool _leq, bool _inserting_zero);

  ConfigState randomize(ConfigState state);

  void reset_zero();

  void run();
  void config_by_str(const std::string &str) {}
};

class LFSRNode : public SNGNode {
public:
  int N;

  // Indicating the LFSR has been configured, otherwise gives an info about
  // using default configurations.
  bool configured = false;
  bool inserting_zero = true;
  bool leq = false;

  LFSR lfsr;
  std::vector<bitstream_list> polynomials;
  std::vector<bitstream_list> seeds;
  std::vector<bitstream_list> scramblings;

  bitstream_list polynomial;
  bitstream_list seed;
  bitstream_list scrambling;

  bitstream_list sequence;
  bitstream_list lookup_table;
  int prev = -1;

  std::unordered_set<std::string> fixed_config;

  void parse_extra();

  LFSRNode(int _input_num, int _output_num, const Config &_conf,
           std::vector<SNGNode *> _master = {});

  void init(const std::vector<std::string> &_extra);

  Node *copy();

  std::string label() { return "LFSR"; };
  std::string codegen_str();

  std::string get_config();

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map);

  std::string verilog_module_init_str();

  void config_by_str(const std::string &str);

  std::string get_config_str();

  // Should be called before randomize()
  void configure(bool _leq, bool _inserting_zero);

  ConfigState copyConf(SNGNode *not_used);

  ConfigState randomize(ConfigState state);

  void reset_zero();

  void run();
};

class SobolNode : public SNGNode {
public:
  int N;

  // Indicating the SobolSNG has been configured, otherwise gives an info about
  // using default configurations.
  bool configured = false;
  bool inserting_zero = true;
  bool leq = false;

  Sobol sobol;
  // std::vector<bitstream_list> polynomials;
  std::vector<bitstream_list> seeds;
  std::vector<bitstream_list> scramblings;

  // bitstream_list polynomial;
  bitstream_list seed;
  bitstream_list scrambling;

  bitstream_list sequence;
  bitstream_list lookup_table;
  int prev = -1;

  std::unordered_set<std::string> fixed_config;

  void parse_extra() {}

  SobolNode(int _input_num, int _output_num, const Config &_conf,
            std::vector<SNGNode *> _master = {});

  void init(const std::vector<std::string> &_extra) {
    extra = _extra;
    parse_extra();
  }

  Node *copy();

  std::string label() { return "Sobol"; };
  std::string codegen_str() { assert(0 && "Not implemented"); }

  std::string get_config();

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map);

  std::string verilog_module_init_str();

  std::string get_config_str();

  // Should be called before randomize()
  void configure(bool _leq, bool _inserting_zero);

  ConfigState copyConf(SNGNode *not_used);

  ConfigState randomize(ConfigState state);

  void reset_zero();

  void run();

  void config_by_str(const std::string &str);
};

#endif
