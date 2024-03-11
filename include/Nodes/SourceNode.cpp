#include "SourceNode.h"
#include "Nodes/OtherNode.h"

SourceNode::SourceNode(int _input_num, int _output_num, const Config &_conf)
    : Node(_input_num, _output_num, _conf) {
  assert(_input_num == 0);
  in_sc_core = false;
  N = conf.bit_width;
  LEN = (int) std::pow(2, N);
  LOWERLIM = conf.scType == unipolar ? 0 : -LEN;
}

Node *SourceNode::copy() {
  Node *new_node =
      new SourceNode(this->input_num, this->output_num, this->conf);
  new_node->id = this->id;
  return new_node;
}

std::string SourceNode::codegen_str() {
  return "    SourceNode *<-name-> = new SourceNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf);" + "\n    inputs.push_back(<-name->);";
}

std::string SourceNode::verilog_module_init_str() {
  std::string out = "\tassign " + verilog_wire_name(this, this->outputs[0]) +
                    " = b_in" + std::to_string(this->id) + ";";
  if (this->next == nullptr)
    out += "\n";

  return out;
}

void SourceNode::reset_zero() {
  clear_ready();
  data = std::vector<int>(1, 0);

#if LOG
  std::cout << "-----"
            << "SourceNode reset_zero(): " << data[0] << "-----" << std::endl;
#endif

  ready = true;
  notify_reset_zero();
  // it should not call next->reset_zero(), because the next source node will
  // do it itself in its run() function.
};
void SourceNode::run() {
  if (conf.optimize & INCREMENTAL) {
    reset_zero();
    for (int x = LOWERLIM; x < LEN; x++) {
      clear_ready();
      data[0] = x;
#if LOG
      std::cout << "-----"
                << "SourceNode:" << data[0] << "-----" << std::endl;
#endif
      ready = true;
      notify();
      // Recursively asssign all SourceNodes
      if (next != nullptr) {
        next->run();
      }
      if (conf.optimize & RUNNING_MINIMAL) {
        if (minNode->get_value()) {
          // std::cout << "skip" << std::endl;
          break;
        }
      }
    }
    clear_ready();
  } else if (conf.optimize & RUNNING_MINIMAL) {
    data = std::vector<int>(1);
    for (int x = 0; x < LEN; x++) {
      clear_ready();
      data[0] = x;
#if LOG
      std::cout << "-----"
                << "SourceNode:" << data[0] << "-----" << std::endl;
#endif
      ready = true;
      notify();
      // Recursively asssign all SourceNodes
      if (next != nullptr) {
        next->run();
      }
      if (minNode->get_value()) {
        // std::cout << "skip" << std::endl;
        break;
      }
    }
    clear_ready();
  } else {
    data = std::vector<int>(1);
    for (int x = LOWERLIM; x < LEN; x++) {
      clear_ready();
      data[0] = x;
#if LOG
      std::cout << "-----"
                << "SourceNode:" << data[0] << "-----" << std::endl;
#endif
      ready = true;

      notify();
      // Recursively asssign all SourceNodes
      if (next != nullptr) {
        next->run();
      }
    }
    clear_ready();
  }
  if (conf.optimize & LOOKUPTABLE) {
    if (!(conf.optimize & INCREMENTAL)) {
      data = std::vector<int>(1);
      for (int x = LOWERLIM; x < LEN; x++) {
        clear_ready();
        data[0] = x;
#if LOG
        std::cout << "-----"
                  << "SourceNode:" << data[0] << "-----" << std::endl;
#endif
        ready = true;
        notify();
        // Recursively asssign all SourceNodes
        if (next != nullptr) {
          next->run();
        }
      }
      clear_ready();
    }
  }
}

RandomSourceNode::RandomSourceNode(int _input_num, int _output_num,
                                   const Config &_conf, int rand_times)
    : SourceNode(_input_num, _output_num, _conf) {
  random_times = rand_times;
}

void RandomSourceNode::init(const std::vector<std::string> &_extra) {
  extra = _extra;
  if (!extra.empty()) {
    random_times = std::stoi(extra[0]);
  }
}

Node *RandomSourceNode::copy() {
  Node *new_node =
      new RandomSourceNode(this->input_num, this->output_num, this->conf);
  new_node->init(this->extra);
  new_node->id = this->id;
  return new_node;
}

std::string RandomSourceNode::codegen_str() {
  return "    RandomSourceNode *<-name-> = new RandomSourceNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf" + ");" + "\n    <-name->->init(<-name->_extra);" +
         (random_times == -1
              ? "\n    inputs.push_back(<-name->);"
              : "\n    inputs.insert(inputs.begin(), <-name->);");
}

void RandomSourceNode::reset_zero() {
  clear_ready();
  data.resize(1);

  data[0] = rand() % (LEN - LOWERLIM) + LOWERLIM;
#if LOG
  std::cout << "-----"
            << "RandomSourceNode reset_zero():" << data[0] << "-----"
            << std::endl;
#endif
  ready = true;
  notify_reset_zero();
  // it should not call next->reset_zero(), because the next source node will
  // do it itself in its run() function.
}

void RandomSourceNode::run() {
  {
    data = std::vector<int>(1);
    if (random_times != -1) {
      for (int i = 0; i < random_times; i++) {
        if (conf.optimize & INCREMENTAL) {
          reset_zero();
          clear_ready();
        } else {
          clear_ready();
          data[0] = rand() % (LEN - LOWERLIM) + LOWERLIM;
#if LOG
          std::cout << "-----"
                    << "RandomSourceNode Head:" << data[0] << "-----"
                    << std::endl;
#endif
        }

        ready = true;
        notify();
        // Recursively asssign all SourceNodes
        if (next != nullptr) {
          next->run();
        }
        if (conf.optimize & RUNNING_MINIMAL) {
          if (minNode->get_value()) {
#if LOG
            std::cout << "-----"
                      << "RandomSourceNode Head: skip!"
                      << "-----" << std::endl;
#endif
            break;
          }
        }
      }
    } else {
      clear_ready();
      data[0] = rand() % (LEN - LOWERLIM) + LOWERLIM;
#if LOG
      std::cout << "-----"
                << "RandomSourceNode:" << data[0] << "-----" << std::endl;
#endif
      ready = true;
      notify();
      // Recursively asssign all SourceNodes
      if (next != nullptr) {
        next->run();
      }
      clear_ready();
    }
  }
}

template <typename T>
std::string ConstantSourceNode::to_string_with_precision(const T a_value,
                                                         const int n) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << a_value;
  return out.str();
}

void ConstantSourceNode::init(const std::vector<std::string> &extra) {
  this->extra = extra;
  value = std::stod(extra[0]);
}

std::string ConstantSourceNode::codegen_str() {
  return "    ConstantSourceNode *<-name-> = new ConstantSourceNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf" + ");" + "\n    <-name->->set_value(" +
         std::to_string(value) + ");" +
         "\n    inputs.insert(inputs.begin(), <-name->);";
}

std::string ConstantSourceNode::verilog_module_init_str() {
  std::string out =
      "\tassign " + verilog_wire_name(this, this->outputs[0]) + " = " +
      std::to_string(this->conf.bit_width) + "'d" +
      std::to_string(conf.scType == unipolar ? int(this->value * LEN)
                                             : int(this->value * LEN / 2)) +
      ";";
  return out;
}

Node *ConstantSourceNode::copy() {
  ConstantSourceNode *new_node =
      new ConstantSourceNode(this->input_num, this->output_num, this->conf);
  new_node->set_value(this->value);
  new_node->id = this->id;
  return new_node;
}

void ConstantSourceNode::reset_zero() {
  clear_ready();
  data.resize(1);

  data[0] = value * LEN;
#if LOG
  std::cout << "-----"
            << "ConstantSourceNode reset_zero():" << data[0] << "-----"
            << std::endl;
#endif
  ready = true;
  notify_reset_zero();
  // it should not call next->reset_zero(), because the next source node will
  // do it itself in its run() function.
}

void ConstantSourceNode::run() {
  data = std::vector<int>(1);
  if (conf.optimize & INCREMENTAL)
    reset_zero();

  clear_ready();
  data[0] = value * LEN;

#if LOG
  std::cout << "-----"
            << "ConstantSourceNode: " << data[0] << "-----" << std::endl;
#endif

  ready = true;
  notify();

  // Recursively asssign all SourceNodes
  if (next != nullptr) {
    next->run();
  }
  clear_ready();
}

UserDefSourceNode::UserDefSourceNode(int _input_num, int _output_num,
                                     const Config &_conf)
    : SourceNode(_input_num, _output_num, _conf) {}

UserDefSourceNode::UserDefSourceNode(int _input_num, int _output_num,
                                     const Config &_conf, int _input_size)
    : SourceNode(_input_num, _output_num, _conf) {
  input_size = _input_size;
}

void UserDefSourceNode::setInput(const std::vector<int> &user_input) {
  user_def_input = user_input;
}

void UserDefSourceNode::init(const std::vector<std::string> &_extra) {
  input_size = user_def_input.size();
}

std::string UserDefSourceNode::codegen_str() {
  return "    UserDefSourceNode *<-name-> = new UserDefSourceNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf" + ");" + "\n    <-name->->init(<-name->_extra);" +
         (input_size == 0 ? "\n    inputs.push_back(<-name->);"
                          : "\n    inputs.insert(inputs.begin(), <-name->);");
}

Node *UserDefSourceNode::copy() {
  // Node *new_node =
  //     new RandomSourceNode(this->input_num, this->output_num, this->conf);
  // new_node->init(this->extra);
  // return new_node;
  assert(0 && "Not implemented");
  return nullptr;
}

void UserDefSourceNode::reset_zero(int index) {
  clear_ready();
  data.resize(1);
  if (index != -1)
    data[0] = user_def_input[index];
  else
    data[0] = rand() % (LEN - LOWERLIM) + LOWERLIM;
#if LOG
  std::cout << "-----"
            << "RandomSourceNode reset_zero():" << data[0] << "-----"
            << std::endl;
#endif
  ready = true;
  notify_reset_zero();
  // it should not call next->reset_zero(), because the next source node will
  // do it itself in its run() function.
}

void UserDefSourceNode::run() {
  if (conf.optimize == NONE || conf.optimize == LOOKUPTABLE) {
    data = std::vector<int>(1);
    clear_ready();
    if (input_size != -1) {
      // first UserDefSourceNode
      for (int i = 0; i < input_size; i++) {
        data[0] = user_def_input[i];
#if LOG
        std::cout << "-----"
                  << "UserDefSourceNode:" << data[0] << "-----" << std::endl;
#endif
        ready = true;
        notify();
        // Recursively asssign all SourceNodes
        if (next != nullptr) {
          next->run();
        }
        clear_ready();
      }
    } else {
      // rest UserDefSourceNode
      data[0] = user_def_input[user_def_input_index % user_def_input.size()];
      user_def_input_index++;

#if LOG
      std::cout << "-----"
                << "UserDefSourceNode:" << data[0] << "-----" << std::endl;
#endif
      ready = true;
      notify();
      // Recursively asssign all SourceNodes
      if (next != nullptr) {
        next->run();
      }
      clear_ready();
    }
  } else if (conf.optimize == INCREMENTAL) {
    if (input_size != -1) {
      for (int i = 0; i < input_size; i++) {
        reset_zero(i);
        clear_ready();
        // data[0] = rand() % LEN+ LOWERLIM;

#if LOG
        std::cout << "-----"
                  << "RandomSourceNode:" << data[0] << "-----" << std::endl;
#endif
        ready = true;
        notify();

        // Recursively asssign all SourceNodes
        if (next != nullptr) {
          next->run();
        }
        clear_ready();
      }
    } else {
      reset_zero(-1);
      clear_ready();
      // data[0] = rand() % LEN+ LOWERLIM;
#if LOG
      std::cout << "-----"
                << "RandomSourceNode:" << data[0] << "-----" << std::endl;
#endif
      ready = true;
      notify();
      // Recursively asssign all SourceNodes
      if (next != nullptr) {
        next->run();
      }
      clear_ready();
    }
  }
}
