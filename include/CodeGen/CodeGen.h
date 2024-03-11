#ifndef CODEGEN_H
#define CODEGEN_H

#include "Modules/SC_core.h"
#include "Nodes/NodeFactory.h"
#include "Utils/Utils.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum Line_type {
  DIGRAPH,
  GV_COMMAND,
  CONFIG,
  COMMENT,
  DEFINITION,
  CONNECTION,
  SUBGRAPH_START,
  SUBGRAPH_END
};

Line_type get_line_type(std::string line);

class Line {
public:
  virtual Line_type get_type() = 0;

  virtual ~Line() = default;

  const std::unordered_set<char> tokens = {'[', ']', '=', ';', ' ', ','};
  const std::vector<std::string> tokens2 = {"//", "/*", "*/", "->"};

  void lex_space(std::string &line);

  virtual std::string lex(std::string &line);

  std::vector<std::string> parse(std::string line);
};

class Configuration : public Line {
public:
  Line_type get_type() { return CONFIG; }

  std::string name;
  std::string value;
  std::vector<std::string> mul_value;
  Configuration(std::string line);
};

class Connection : public Line {
public:
  Line_type get_type() { return CONNECTION; }

  std::string source;
  std::string destination;
  std::vector<std::pair<std::string, std::string>> attributes;
  Connection(std::string line);
};

class Digraph : public Line {
public:
  Line_type get_type() { return DIGRAPH; }

  std::string graph_name;

  Digraph(std::string line);
};

class Definition : public Line {
public:
  Line_type get_type() { return DEFINITION; }

  std::string name;
  std::string node_type;
  std::vector<std::pair<std::string, std::string>> attributes;
  std::vector<std::string> extra;

  Definition(std::string line);
};

void emit_header(std::ofstream &output_file);
void emit_before(std::ofstream &output_file,
                 const std::vector<Config> &configs);
void emit_after(std::ofstream &output_file, const std::vector<Config> &configs);

// Split the line by space
std::vector<std::string> parse_line(std::string line);
std::vector<std::unique_ptr<Line>> parse_input_file(std::ifstream &input_file);
Node *parse_node(
    const Definition *def,
    const std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                                    std::vector<std::string>>>
        &name_map,
    const Config &conf);
int find_in_index(
    const std::string &in_node, const std::string &out_node,
    const std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                                    std::vector<std::string>>>
        &name_map);
void parse_definitions(
    const std::vector<std::unique_ptr<Line>> &inputs,
    const std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                                    std::vector<std::string>>>
        &name_map,
    std::unordered_map<std::string, Node *> &node_map, const Config &conf);
void parse_connections(
    const std::vector<std::unique_ptr<Line>> &inputs,
    std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                              std::vector<std::string>>>
        &name_map);
/**
 * @brief Set the config Object
 *  If the config only has one value, ignore the index variable.
 *  If the config has multi-values, set corresponding value according to index.
 *
 * @param conf
 * @param index
 */
void set_config(Config &conf, const Configuration *configuration, int index);
void config_helper(const Configuration *configuration,
                   std::vector<Config> &configs);
void parse_inputs(
    const std::vector<std::unique_ptr<Line>> &inputs,
    std::vector<Config> &configs,
    std::unordered_map<std::string, Node *> &node_map,
    std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                              std::vector<std::string>>>
        &name_map);

void parse_file(
    const std::string &file_path, const Config &conf,
    std::unordered_map<std::string, Node *> &node_map,
    std::unordered_map<std::string, std::pair<std::vector<std::string>,
                                              std::vector<std::string>>>
        &name_map);

class Subgraph_start : public Line {
public:
  Line_type get_type() { return SUBGRAPH_START; }

  std::string name;

  Subgraph_start(std::string line);
};

class Subgraph_end : public Line {
public:
  Line_type get_type() { return SUBGRAPH_END; }

  Subgraph_end(std::string line);
};

#endif