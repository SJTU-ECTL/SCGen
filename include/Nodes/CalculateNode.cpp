#include "CalculateNode.h"

CalculateNode::CalculateNode(int _input_num, int _output_num,
                             const Config &_conf)
    : Node(_input_num, _output_num, _conf) {
  N = conf.bit_width;
  LEN = (int) std::pow(2, N);
  in_sc_core = false;
  LOWERLIM = conf.scType == unipolar ? 0 : -LEN;
}
void CalculateNode::set_fn(
    std::function<double(const std::vector<double> &)> _fn) {
  fn = _fn;
}

std::string MAENode::codegen_str() {
  return "    MAENode *<-name-> = new MAENode(" + std::to_string(input_num) +
         ", " + std::to_string(output_num) + ", conf);" +
         "\n    <-name->->set_fn(fn);";
}

Node *MAENode::copy() {
  MAENode *new_node =
      new MAENode(this->input_num, this->output_num, this->conf);
  new_node->set_fn(this->fn);
  new_node->id = this->id;
  return new_node;
}

void MAENode::reset_zero() {
  if (ready || !is_ready())
    return;
#if LOG
  std::cout << "MAENode reset_zero()" << std::endl;
#endif
  ready = true;
  notify_reset_zero();
}

void MAENode::run() {
  if (ready || !is_ready())
    return;
  std::vector<double> input_values(input_num - 1);
  for (int i = 0; i < input_num - 1; i++) {
    input_values[i] = (double) inputs[i]->data[0] / LEN;
  }
  double real_value = fn(input_values);
  double simulated_value = (double) inputs[input_num - 1]->data[0] / LEN;

  MAEs.push_back(std::abs(real_value - simulated_value));

  sum += std::abs(real_value - simulated_value);
#if LOG
  std::cout << "real_value: " << real_value << " --- "
            << "simulated_value: " << simulated_value << std::endl;
  std::cout << "MAENode: " << simulated_value << std::endl;
#endif

  ready = true;
  notify();
}

void MAENode::reset() {
  MAEs.clear();
  sum = 0;
}

double MAENode::get_sum() { return sum; }

double MAENode::get_MAE() {
#if LOG
  std::cout << "MAE size(): " << MAEs.size() << std::endl;
#endif
  return sum / MAEs.size();
}

double MAENode::get_value() { return get_MAE(); }

std::string MAENode::get_result_str() {
  return "MAENode with value " + std::to_string(get_value());
}

MSENode::MSENode(int _input_num, int _output_num, const Config &_conf)
    : CalculateNode(_input_num, _output_num, _conf) {}

std::string MSENode::codegen_str() {
  return "    MSENode *<-name-> = new MSENode(" + std::to_string(input_num) +
         ", " + std::to_string(output_num) + ", conf);" +
         "\n    <-name->->set_fn(fn);" +
         //  "\n    calculates.push_back(<-name->);";
         "";
}

Node *MSENode::copy() {
  MSENode *new_node =
      new MSENode(this->input_num, this->output_num, this->conf);
  new_node->set_fn(this->fn);
  new_node->id = this->id;
  return new_node;
}

void MSENode::reset_zero() {
  if (ready || !is_ready())
    return;

#if LOG
  std::cout << "MSENode reset_zero()" << std::endl;
#endif

  ready = true;
  notify_reset_zero();
}

void MSENode::run() {
  if (ready || !is_ready())
    return;
  std::vector<double> input_values(input_num - 1);

  for (int i = 0; i < input_num - 1; i++) {
    input_values[i] = (double) inputs[i]->data[0] / LEN;
  }

  double real_value = fn(input_values);
  double simulated_value = (double) inputs[input_num - 1]->data[0] / LEN;
  MSEs.push_back(std::pow(real_value - simulated_value, 2));
  sum += std::pow(real_value - simulated_value, 2);
#if LOG
  std::cout << "real_value: " << real_value << " --- "
            << "simulated_value: " << simulated_value << std::endl;
  std::cout << "MSENode: " << simulated_value << std::endl;
#endif

  ready = true;
  notify();
}

void MSENode::reset() {
  MSEs.clear();
  sum = 0;
}

double MSENode::get_sum() { return sum; }

double MSENode::get_MSE() {
#if LOG
  std::cout << "MSE size(): " << MSEs.size() << std::endl;
#endif
  return sum / MSEs.size();
}

double MSENode::get_value() { return get_MSE(); }

std::string MSENode::get_result_str() {
  return "MSENode with value " + std::to_string(get_value());
}

MinNode::MinNode(int _input_num, int _output_num, const Config &_conf)
    : CalculateNode(_input_num, _output_num, _conf) {}

std::string MinNode::codegen_str() {
  return "    MinNode *<-name-> = new MinNode(" + std::to_string(input_num) +
         ", " + std::to_string(output_num) + ", conf);" +
         "\n    calculates.push_back(<-name->);";
}

Node *MinNode::copy() {
  Node *node = new MinNode(this->input_num, this->output_num, this->conf);
  node->id = this->id;
  return node;
}

void MinNode::reset_zero() {
  if (ready || !is_ready())
    return;
#if LOG
  std::cout << "MinNode reset_zero()" << std::endl;
#endif
  skip = false;

  ready = true;
  notify_reset_zero();
}

void MinNode::clear() {
  min_value = 1000000;
  min_sum = 1000000;
}

void MinNode::run() {
  if (ready || !is_ready())
    return;

  if (conf.optimize & RUNNING_MINIMAL) {
    if (min_sum < ((CalculateNode *) inputs[0])->get_sum()) {
#if LOG
      std::cout << "MinNode skip!" << std::endl;
#endif
      skip = true;
    }
  }
#if LOG
  std::cout << "MinNode: " << std::endl;
#endif

  ready = true;
  notify();
}

void MinNode::reset() { skip = false; }

double MinNode::get_sum() { return -1; }

void MinNode::finish() {
  double value = ((CalculateNode *) inputs[0])->get_value();

  if (min_value > value) {
    min_value = value;
    min_sum = ((CalculateNode *) inputs[0])->get_sum();
    min_configs = "";
    for (int i = 1; i < input_num; i++) {
      min_configs +=
          "\n" + std::to_string(i) + ": " + inputs[i]->get_config_str();
    }
  }
}

double MinNode::get_value() { return skip ? 1 : 0; }

std::string MinNode::get_result_str() {
  return "MinNode of " + inputs[0]->get_label() + " with value " +
         std::to_string(min_value) + " and config: " + min_configs;
}

AverageNode::AverageNode(int _input_num, int _output_num, const Config &_conf)
    : CalculateNode(_input_num, _output_num, _conf) {}

std::string AverageNode::codegen_str() {
  return "    AverageNode *<-name-> = new AverageNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf);" + "\n    calculates.push_back(<-name->);";
}

Node *AverageNode::copy() {
  Node *node = new AverageNode(this->input_num, this->output_num, this->conf);
  node->id = this->id;
  return node;
}

void AverageNode::reset_zero() {
  if (ready || !is_ready())
    return;
#if LOG
  std::cout << "AverageNode reset_zero()" << std::endl;
#endif
  ready = true;
  notify_reset_zero();
}

void AverageNode::run() {
  if (ready || !is_ready())
    return;

#if LOG
  std::cout << "AverageNode: " << std::endl;
#endif

  ready = true;
  notify();
}

void AverageNode::reset() {}

double AverageNode::get_sum() { return -1; }

void AverageNode::finish() {
  double value = ((CalculateNode *) inputs[0])->get_value();

  sum += value;
  num++;
}

double AverageNode::get_value() { return 0; }

std::string AverageNode::get_result_str() {
  return "AverageNode of " + inputs[0]->get_label() + " with value " +
         std::to_string(sum / num);
}