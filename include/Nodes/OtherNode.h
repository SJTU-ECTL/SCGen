/**
 * @brief
 *     Other Nodes that can be fit in none of the files.
 *     Temporarily put the DestNode here.
 */

#ifndef OTHERNODE_H
#define OTHERNODE_H

#include "Node.h"
#include "Utils/Utils.h"
#include <exception>
#include <fstream>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

// TODO: unify this one with the CounterNode in BinaryNode.h.
class DestNode : public Node {
private:
  bitstream_list previous;

public:
  DestNode(int _input_num, int _output_num, const Config &_conf);

  std::string label() { return "Dest"; }

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map);

  std::string
  verilog_codegen_multi_output(std::unordered_map<Node *, int> node_index_map);

  std::string codegen_str();

  virtual Node *copy();

  void reset_zero();

  void run();
};

class BinaryDestNode : public DestNode {
public:
  // DestNode() : Node(1, 0) {}
  BinaryDestNode(int _input_num, int _output_num, const Config &_conf);

  virtual Node *copy();

  std::string label() { return "BinaryDest"; }

  std::string codegen_str();

  std::string
  verilog_codegen_str(std::unordered_map<Node *, int> node_index_map);

  std::string
  verilog_codegen_multi_output(std::unordered_map<Node *, int> node_index_map);

  void run();
};

class ImageDestNode : public DestNode {
private:
  bitstream_list previous;
  int kernel_width;
  int kernel_height;
  int pos_x;
  int pos_y;
  cv::Mat original_image;
  cv::Mat new_image;
  int current_x;
  int current_y;

public:
  ImageDestNode(int _input_num, int _output_num, const Config &_conf)
      : DestNode(_input_num, _output_num, _conf) {}

  std::string label() { return "Image_output"; }
  std::string codegen_str() {
    return "    ImageDestNode *<-name-> = new ImageDestNode(" +
           std::to_string(input_num) + ", " + std::to_string(output_num) +
           ", conf" + ");" + "\n    outputs.push_back(<-name->);";
  }

  void init(const std::vector<std::string> &extra) {
    kernel_width = std::stoi(extra[0]);
    kernel_height = std::stoi(extra[1]);
    pos_x = std::stoi(extra[2]);
    pos_y = std::stoi(extra[3]);
    original_image = cv::imread(extra[4], cv::IMREAD_UNCHANGED);
    new_image = cv::Mat::zeros(
        cv::Size(original_image.cols, original_image.rows), CV_8UC1);
    current_x = -1;
    current_y = -1;
  }

  void reset_zero() {
    if (ready || !is_ready())
      return;

    update.clear();

    data = std::vector<int>(1);
    data[0] = count_arr(inputs[0]->data, 0, inputs[0]->data.size());

    previous = inputs[0]->data;

#if LOG
    std::cout << "ImageDestNode reset_zero(): " << data[0] << std::endl;
#endif

    ready = true;
    notify_reset_zero();
  }

  void run() {
    if (ready || !is_ready())
      return;
    if (conf.optimize & INCREMENTAL) {
      int pre = data[0];
      update.clear();
      for (const auto &up : inputs[0]->update) {
        if (inputs[0]->data[up] != previous[up]) {
          data[0] += inputs[0]->data[up] - previous[up];
          previous[up] = inputs[0]->data[up];
        }
      }
      if (pre != data[0])
        update.insert(0);
    }

    if (conf.optimize == NONE) {
      data.resize(1);
      data[0] = inputs[0]->data[0];
      if (current_x == -1 && current_y == -1) {
        current_x = pos_x;
        current_y = pos_y;
      }
      new_image.at<uchar>(current_x, current_y) = data[0];
      current_y++;
      if (current_y == new_image.cols - (kernel_width - pos_y) + 1) {
        current_y = pos_y;
        current_x++;
      }
    }
    if (conf.optimize & LOOKUPTABLE) {
      if (!(conf.optimize & INCREMENTAL)) {
        data.resize(1);
        data[0] = count_arr(inputs[0]->data, 0, inputs[0]->data.size());
      }
    }
#if LOG
    std::cout << "ImageDestNode: " << data[0] << std::endl;
#endif

    ready = true;
    notify();
  }

  virtual void finish() {
    std::cout << "ImageDestNode finished" << std::endl;
    cv::imwrite("./image_output.png", new_image);
  };
};

#endif