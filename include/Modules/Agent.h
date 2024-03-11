#ifndef AGENT_H
#define AGENT_H

#include <string>
namespace SC_sim {

class Var;

struct AgentConfig {
  int NO_IMPROVE_BOUND;
};

class MCTSAgent {
public:
  static std::pair<double, std::string> run(Var *var, AgentConfig agent_config);
};
class SAAgent {
public:
  static std::pair<double, std::string> run(Var *var, AgentConfig agent_config);
};

class SA2Agent {
public:
  static std::pair<double, std::string> run(Var *var, AgentConfig agent_config);
};

class GAAgent {
public:
  static std::pair<double, std::string> run(Var *var, AgentConfig agent_config);
};

class GA2Agent {
public:
  static std::pair<double, std::string> run(Var *var, AgentConfig agent_config);
};

class RandomAgent {
public:
  static std::pair<double, std::string> run(Var *var, AgentConfig agent_config);
};

class DescentAgent {
public:
  static std::pair<double, std::string> run(Var *var, AgentConfig agent_config);
};
}   // namespace SC_sim

#endif