/**
 * @brief
 *     The definiton and implementation of SourceNodes.
 *     SourceNode outputs numbers from 0 to 2^n - 1.
 *     RandomSourceNode outputs numbers randomly in range [0, 2^n - 1].
 */

#ifndef SOURCENODE_H
#define SOURCENODE_H

#include "Node.h"
#include "Nodes/CalculateNode.h"
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>

/**
 * @brief SourceNode represents the number source, that gives current number to
 * SNGs.
 */
class SourceNode : public Node {
protected:
  int N;
  SourceNode *next = nullptr;
  int LEN;
  int LOWERLIM;

public:
  SourceNode(int _input_num, int _output_num, const Config &_conf);

  CalculateNode *minNode = nullptr;

  void set_next_node(SourceNode *_next) { next = _next; }
  SourceNode *get_next_node() { return next; }

  virtual Node *copy() override;

  std::string label() override { return "Source"; }
  std::string codegen_str() override;

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) override {
    return "";
  }
  std::string verilog_module_init_str() override;

  void reset_zero() override;

  virtual void run() override;
};

class RandomSourceNode : public SourceNode {
  int random_times = -1;

public:
  RandomSourceNode(int _input_num, int _output_num, const Config &_conf)
      : SourceNode(_input_num, _output_num, _conf) {}

  void init(const std::vector<std::string> &_extra);
  RandomSourceNode(int _input_num, int _output_num, const Config &_conf,
                   int rand_times);

  std::string label() { return "RandomSource"; }

  virtual Node *copy();

  std::string codegen_str();

  void reset_zero();

  void run() override;
};

class ConstantSourceNode : public SourceNode {
private:
  double value;

  template <typename T>
  std::string to_string_with_precision(const T a_value, const int n = 6);

public:
  ConstantSourceNode(int _input_num, int _output_num, const Config &_conf)
      : SourceNode(_input_num, _output_num, _conf) {}

  void init(const std::vector<std::string> &extra);

  void set_value(double _value) { value = _value; }

  std::string label() { return "ConstantSource"; }
  std::string codegen_str();

  std::string verilog_module_init_str() override;

  virtual Node *copy();

  void reset_zero();

  void run() override;
};

class UserDefSourceNode : public SourceNode {
  int input_size = -1;
  int user_def_input_index = 0;
  std::vector<int> user_def_input;

public:
  UserDefSourceNode(int _input_num, int _output_num, const Config &_conf);
  UserDefSourceNode(int _input_num, int _output_num, const Config &_conf,
                    int _input_size);

  void setInput(const std::vector<int> &user_input);

  void init(const std::vector<std::string> &_extra);

  std::string label() { return "UserDefSource"; }

  std::string codegen_str();

  // std::string
  // verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
  //   std::string out = "\tassign" + verilog_codegen_io_str(node_index_map) +
  //                     "= bin_" + std::to_string(node_index_map[this]) + ";";
  //   return out;
  //   //		return std::string("\tSource #(.bit_width(bit_width)) ")
  //   + "Source_"
  //   //+ std::to_string(node_index_map[this])
  //   +
  //   "(\n\t\tbin_"
  //   //+std::to_string(node_index_map[this]) + ",\n"+
  //   //
  //   verilog_codegen_io_str(node_index_map)
  //   +
  //   "\t\tclk\n"
  //   +
  //   //"\t);\n";
  // }
  virtual Node *copy();

  void reset_zero(int index = -1);

  void run();
};

class ImageSourceNode : public SourceNode {
private:
  int kernel_width;
  int kernel_height;
  int pos_x;
  int pos_y;
  cv::Mat image;
  int current_x;
  int current_y;

public:
  ImageSourceNode(int _input_num, int _output_num, const Config &_conf)
      : SourceNode(_input_num, _output_num, _conf) {}

  void init(const std::vector<std::string> &extra) {
    this->extra = extra;
    kernel_width = std::stoi(extra[0]);
    kernel_height = std::stoi(extra[1]);
    pos_x = std::stoi(extra[2]);
    pos_y = std::stoi(extra[3]);
    image = cv::imread(extra[4], cv::IMREAD_UNCHANGED);
    current_x = -1;
    current_y = -1;
  }

  std::string label() { return "ImageSourceNode"; }
  std::string codegen_str() {
    return "    ImageSourceNode *<-name-> = new ImageSourceNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");" + "\n    <-name->->init(<-name->_extra);" +
           "\n    inputs.insert(inputs.begin(), <-name->);";
  }

  virtual Node *copy() {
    ImageSourceNode *new_node =
        new ImageSourceNode(this->input_num, this->output_num, this->conf);
    new_node->init(this->extra);
    new_node->id = this->id;
    return new_node;
  }

  void reset_zero() {
    clear_ready();
    data.resize(1);

    assert(0);

#if LOG
    std::cout << "-----"
              << "ImageSourceNode reset_zero():" << data[0] << "-----"
              << std::endl;
#endif
    ready = true;
    notify_reset_zero();
    // it should not call next->reset_zero(), because the next source node will
    // do it itself in its run() function.
  }

  void run() {
    data = std::vector<int>(1);
    if (conf.optimize & INCREMENTAL)
      reset_zero();

    clear_ready();

    // Head source node
    if (pos_x == 0 && pos_y == 0) {
      // std::cout << "head source node" << std::endl;
      while (current_x < image.rows - (kernel_height - pos_x) + 1) {
        clear_ready();
        if (current_x == -1 && current_y == -1) {
          current_x = pos_x;
          current_y = pos_y;
        }
        // std::cout << "--------------------" << std::endl;
        // std::cout << current_x << "," << current_y << " reads " << data[0]
        //           << std::endl;
        data[0] = image.at<uchar>(current_x, current_y);
        current_y++;
        if (current_y == image.cols - (kernel_width - pos_y) + 1) {
          current_x++;
          current_y = pos_y;
        }

        ready = true;
        notify();

        // Recursively asssign all SourceNodes
        if (next != nullptr) {
          next->run();
        }
        clear_ready();
      }
    } else { // Other nodes
      if (current_x == -1 && current_y == -1) {
        current_x = pos_x;
        current_y = pos_y;
      }
      data[0] = image.at<uchar>(current_x, current_y);
      // std::cout << current_x << "," << current_y << " reads " << data[0]
      //           << std::endl;
      // data[0] = 0;
      // current_x++;
      current_y++;
      if (current_y == image.cols - (kernel_width - pos_y) + 1) {
        current_x++;
        current_y = pos_y;
      }
    }

#if LOG
    std::cout << "-----"
              << "ImageSourceNode:" << data[0] << "-----" << std::endl;
#endif

    ready = true;
    notify();

    // Recursively asssign all SourceNodes
    if (next != nullptr) {
      next->run();
    }
    clear_ready();
  }
};

#endif
