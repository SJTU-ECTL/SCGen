#ifndef CALCULATE_H
#define CALCULATE_H

#include "Node.h"
#include <functional>

/**
 * @brief The last element in the inputs vector is the output node.
 */
class CalculateNode : public Node {
protected:
  int N;
  int LEN;
  int LOWERLIM;
  std::function<double(const std::vector<double> &)> fn;

  CalculateNode(int _input_num, int _output_num, const Config &_conf);

public:
  void set_fn(std::function<double(const std::vector<double> &)> _fn);

  virtual double get_value() = 0;
  virtual double get_sum() = 0;
  virtual std::string get_result_str() = 0;

  bool verilog_has_instance() override { return false; }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) override {
    return "";
  }
  std::string verilog_codegen_multi_output(
      std::unordered_map<Node *, int> node_index_map) override {
    return "";
  }

  virtual Node *copy() override = 0;
};

class MAENode : public CalculateNode {
private:
  std::vector<double> MAEs;
  double sum;

public:
  MAENode(int _input_num, int _output_num, const Config &_conf)
      : CalculateNode(_input_num, _output_num, _conf) {}

  std::string label() { return "MAE"; }
  std::string codegen_str();

  virtual Node *copy();

  void reset_zero();

  void run();

  void reset();

  double get_sum();

  double get_MAE();

  double get_value();

  std::string get_result_str();
};

class MSENode : public CalculateNode {
private:
  std::vector<double> MSEs;
  double sum;

public:
  MSENode(int _input_num, int _output_num, const Config &_conf);

  std::string label() { return "MSE"; }
  std::string codegen_str();

  virtual Node *copy();

  void reset_zero();

  void run();

  void reset();

  double get_sum();

  double get_MSE();

  double get_value();

  std::string get_result_str();
};

/**
 * @brief First Node in inputs are the calculate node, and the rest are nodes
 * with configuration
 */
class MinNode : public CalculateNode {
private:
  double min_value = 1000000;
  double min_sum = 1000000;
  bool skip = false;
  std::string min_configs;

public:
  MinNode(int _input_num, int _output_num, const Config &_conf);

  std::string label() { return "MIN"; }
  std::string codegen_str();

  virtual Node *copy();

  void reset_zero();

  void clear();

  void run();

  void reset();
  double get_sum();

  void finish();

  double get_value();

  std::string get_result_str();
};

class AverageNode : public CalculateNode {
private:
  double sum = 0;
  int num = 0;

public:
  AverageNode(int _input_num, int _output_num, const Config &_conf);

  std::string label() { return "AVERAGE"; }
  std::string codegen_str();

  virtual Node *copy();

  void reset_zero();

  void run();

  void reset();
  double get_sum();

  void finish();

  double get_value();

  std::string get_result_str();
};

#endif