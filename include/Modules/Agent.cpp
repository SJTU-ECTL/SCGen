#include "Agent.h"
#include "Nodes/SNGNode.h"
#include "Var.h"
#include <vector>

// FIXME: Consider master[1], although not used yet. (only for CI fig[6])
void SNG_copy_config(SC_sim::Var *var, std::vector<ConfigState> &states) {
  int SNG_num = var->graph.SNGs.size();

  for (int i = 0; i < SNG_num; i++) {
    const auto &sng = var->graph.SNGs[i];
    if (!sng->master.empty()) {
      states[i] = sng->copyConf(sng->master[0]);
    }
  }
}

std::vector<ConfigState> random_SNGs(SC_sim::Var *var) {
  int SNG_num = var->graph.SNGs.size();

  std::vector<ConfigState> states(SNG_num);
  // The order of sng to randomize matters in order to keep the
  // randomness.
  for (int i = 0; i < SNG_num; i++) {
    const auto &sng = var->graph.SNGs[i];
    if (sng->master.empty())
      states[i] = sng->randomize();
  }

  SNG_copy_config(var, states);
  return states;
}

std::pair<double, std::string>
SC_sim::RandomAgent::run(Var *var, AgentConfig agent_config) {
  double min_value = 10000;
  std::string min_result = "";
  int no_improve_time = 0;

  var->simulate_reset();

  for (int i = 0; i < var->conf.rand_times; ++i) {

    auto states = random_SNGs(var);

    auto result_pair = var->simulate_once(true);

    if (min_value > result_pair.first) {
      min_value = result_pair.first;
      min_result = result_pair.second;
      no_improve_time = 0;
    } else {
      no_improve_time++;
      if (var->conf.rand_till_converge &&
          no_improve_time > agent_config.NO_IMPROVE_BOUND) {
        break;
      }
    }
    // std::cout << min_value << " " << result_pair.first << std::endl;
  }
  return {min_value, min_result};
}

std::pair<double, std::string> SC_sim::SAAgent::run(Var *var,
                                                    AgentConfig agent_config) {
  double min_value = 10000;
  std::string min_result = "";
  double ret_value;
  int no_improve_time = 0;

  const int number_of_runs = 2;
  const int rand_per_run = var->conf.rand_times / number_of_runs;

  std::vector<ConfigState> states;

  for (int run_index = 0; run_index < number_of_runs; run_index++) {
    // std::cout << "Run index: " << run_index << std::endl;

    no_improve_time = 0;
    double min_value_single_run = 10000;
    std::string min_result_single_run = "";

    var->simulate_reset();
    states = random_SNGs(var);

    auto result_pair = var->simulate_once(true);
    min_value_single_run = result_pair.first;
    min_value = std::min(min_value, min_value_single_run);

    double T0 = 2 * min_value_single_run;
    double temperature = T0;

    for (int i = 0; i < rand_per_run; ++i) {
      temperature = T0 * (1 - (double) i / rand_per_run);
      // temperature = 0.995 * temperature;
      // std::cout << "Temperature: " << temperature << std::endl;

      // Random select one sng
      int rand_index;
      while (true) {
        rand_index = rand() % states.size();
        if (var->graph.SNGs[rand_index]->master.empty())
          break;
      }

      ConfigState current_state = states[rand_index];
      ConfigState new_state = current_state;

      int change = 0;
      int distance = 100 * (1 - (double) i / rand_per_run) + 1;
      while (change == 0)
        change = rand() % (2 * distance) - distance;

      int parameter_index = rand() % 3;
      // double prob = (double) rand() / RAND_MAX;
      if (parameter_index == 0) {
        int poly_change = 0;
        while (poly_change == 0)
          poly_change = rand() % 10 - 3;

        new_state.poly_index = current_state.poly_index + poly_change;
      } else if (parameter_index == 1) {
        new_state.seed_index = current_state.seed_index + change;
      } else if (parameter_index == 2) {
        new_state.scram_index = current_state.scram_index + change;
      }

      new_state = var->graph.SNGs[rand_index]->randomize(new_state);

      SNG_copy_config(var, states);

      auto result_pair = var->simulate_once(true);
      ret_value = result_pair.first;

      if (min_value_single_run >= ret_value) {
        min_value_single_run = ret_value;
        min_result_single_run = result_pair.second;
        states[rand_index] = new_state;

        SNG_copy_config(var, states);
      } else {
        // Accept the result with probability
        double rand_temp = (double) rand() / RAND_MAX;
        // std::cout << "ret_value: " << ret_value
        //           << " min_value_single_run: " << min_value_single_run
        //           << std::endl;
        // std::cout << "Compare value: "
        //           << std::exp(-(ret_value - min_value_single_run) /
        //           temperature)
        //           << std::endl;
        if (rand_temp <
            std::exp(-(ret_value - min_value_single_run) / temperature)) {
          min_value_single_run = ret_value;
          min_result_single_run = result_pair.second;
          // std::cout << "Probalisticly select value: " << min_value_single_run
          //           << std::endl;
          states[rand_index] = new_state;

          SNG_copy_config(var, states);
        }
      }
      if (min_value > min_value_single_run) {
        min_value = min_value_single_run;
        min_result = min_result_single_run;
        no_improve_time = 0;
      } else {
        no_improve_time++;
        if (var->conf.rand_till_converge &&
            no_improve_time > agent_config.NO_IMPROVE_BOUND) {
          break;
        }
      }
      // std::cout << min_value << " " << ret_value << std::endl;
    }
  }
  return {min_value, min_result};
}

// Each RNS is a gene
std::pair<double, std::string> SC_sim::GAAgent::run(Var *var,
                                                    AgentConfig agent_config) {
  double min_value = 10000;
  std::string min_result = "";
  int no_improve_time = 0;

  bool no_improve = false;

  const int INDIVIDUAL_COUNT = 50;
  const double TOP_RATE = 0.2;
  const double MUTATE_RATE = 0.8;
  const double MUTATE_RATE_PER_SNG = 0.3;

  const int ITERATION_NUM = var->conf.rand_times / INDIVIDUAL_COUNT;
  const int TOP_NUM = INDIVIDUAL_COUNT * TOP_RATE;

  const int SNG_NUM = var->graph.SNGs.size();

  std::vector<std::vector<ConfigState>> population(INDIVIDUAL_COUNT);
  std::vector<std::pair<std::vector<ConfigState>, double>> results(
      INDIVIDUAL_COUNT);

  var->simulate_reset();

  std::vector<std::unordered_map<SNGNode *, int>> SNG_map(INDIVIDUAL_COUNT);
  // Randomize population
  for (int i = 0; i < INDIVIDUAL_COUNT; i++) {
    population[i] = random_SNGs(var);
  }

  for (int iter = 0; iter < ITERATION_NUM; iter++) {
    // Simulate individuals in each iteration
    for (int i = 0; i < INDIVIDUAL_COUNT; i++) {
      for (int index = 0; index < var->graph.SNGs.size(); index++) {
        const auto &sng = var->graph.SNGs[index];
        if (sng->master.empty()) {
          population[i][index] = sng->randomize(population[i][index]);
        }
      }

      SNG_copy_config(var, population[i]);

      auto result_pair = var->simulate_once(true);
      double result = result_pair.first;
      results[i] = std::pair(population[i], result);

      if (min_value > result) {
        min_value = result;
        min_result = result_pair.second;
        no_improve_time = 0;
      } else {
        no_improve_time++;
        if (var->conf.rand_till_converge &&
            no_improve_time > agent_config.NO_IMPROVE_BOUND) {
          no_improve = true;
          break;
        }
      }
      // std::cout << min_value << " " << result << std::endl;
    }
    if (no_improve)
      break;

    // Sort by MAE
    sort(results.begin(), results.end(),
         [](std::pair<std::vector<ConfigState>, double> a,
            std::pair<std::vector<ConfigState>, double> b) {
           return a.second < b.second;
         });

    // Update population according to results
    for (int i = 0; i < TOP_NUM; i++) {
      population[i] = results[i].first;
    }

    // Generate next iteration
    // TOP_NUM individuals remain the same
    for (int i = TOP_NUM; i < INDIVIDUAL_COUNT; i++) {
      double prob = (double) rand() / RAND_MAX;
      if (prob < (1 - MUTATE_RATE)) {
        // Crossover
        // Randomly select two in the TOP_NUM
        int rand_a = rand() % TOP_NUM;
        int rand_b = rand() % TOP_NUM;
        std::vector<ConfigState> ret_a = population[rand_a];
        std::vector<ConfigState> ret_b = population[rand_b];
        int crossover_position = rand() % SNG_NUM;
        for (int j = 0; j < SNG_NUM; j++) {
          if (j >= crossover_position) {
            std::swap(ret_a[j], ret_b[j]);
          }
        }
        // Add to population
        population[i] = ret_a;
        i++;
        if (i < INDIVIDUAL_COUNT)
          population[i] = ret_b;
      } else {
        // Mutation
        int rand_a = rand() % TOP_NUM;
        std::vector<ConfigState> individual = population[rand_a];
        if ((double) rand() / RAND_MAX < 0.2) {
          for (int j = 0; j < individual.size(); j++) {
            std::swap(individual[j], individual[individual.size() - 1 - j]);
          }
        } else {
          for (int j = 0; j < SNG_NUM; j++) {
            if ((double) rand() / RAND_MAX < MUTATE_RATE_PER_SNG) {
              // Mutate individual SNG setting
              int parameter_index = rand() % 3;
              if (parameter_index == 0) {
                individual[j].poly_index = -1;
              } else if (parameter_index == 1) {
                individual[j].seed_index = -1;
              } else if (parameter_index == 2) {
                individual[j].seed_index = -1;
              }
            }
          }
        }
        // Add to population
        population[i] = individual;
      }
    }
  }
  // std::cout << min_result << std::endl;

  return {min_value, min_result};
}

std::pair<double, std::string> SC_sim::GA2Agent::run(Var *var,
                                                    AgentConfig agent_config) {
  double min_value = 10000;
  std::string min_result = "";
  int no_improve_time = 0;

  bool no_improve = false;

  const int INDIVIDUAL_COUNT = 50;
  const double TOP_RATE = 0.2;
  const double MUTATE_RATE = 0.8;
  const double MUTATE_RATE_PER_SNG = 0.3;

  const int ITERATION_NUM = var->conf.rand_times / INDIVIDUAL_COUNT;
  const int TOP_NUM = INDIVIDUAL_COUNT * TOP_RATE;

  const int SNG_NUM = var->graph.SNGs.size();

  std::vector<std::vector<ConfigState>> population(INDIVIDUAL_COUNT);
  std::vector<std::pair<std::vector<ConfigState>, double>> results(
      INDIVIDUAL_COUNT);

  var->simulate_reset();

  std::vector<std::unordered_map<SNGNode *, int>> SNG_map(INDIVIDUAL_COUNT);
  // Randomize population
  for (int i = 0; i < INDIVIDUAL_COUNT; i++) {
    population[i] = random_SNGs(var);
  }

  for (int iter = 0; iter < ITERATION_NUM; iter++) {
    // Simulate individuals in each iteration
    for (int i = 0; i < INDIVIDUAL_COUNT; i++) {
      for (int index = 0; index < var->graph.SNGs.size(); index++) {
        const auto &sng = var->graph.SNGs[index];
        if (sng->master.empty()) {
          population[i][index] = sng->randomize(population[i][index]);
        }
      }

      SNG_copy_config(var, population[i]);

      auto result_pair = var->simulate_once(true);
      double result = result_pair.first;
      results[i] = std::pair(population[i], result);

      if (min_value > result) {
        min_value = result;
        min_result = result_pair.second;
        no_improve_time = 0;
      } else {
        no_improve_time++;
        if (var->conf.rand_till_converge &&
            no_improve_time > agent_config.NO_IMPROVE_BOUND) {
          no_improve = true;
          break;
        }
      }
      // std::cout << min_value << " " << result << std::endl;
    }
    if (no_improve)
      break;

    // Sort by MAE
    sort(results.begin(), results.end(),
         [](std::pair<std::vector<ConfigState>, double> a,
            std::pair<std::vector<ConfigState>, double> b) {
           return a.second < b.second;
         });

    // Update population according to results
    for (int i = 0; i < TOP_NUM; i++) {
      population[i] = results[i].first;
    }

    // Generate next iteration
    // TOP_NUM individuals remain the same
    for (int i = TOP_NUM; i < INDIVIDUAL_COUNT; i++) {
      double prob = (double) rand() / RAND_MAX;
      if (prob < (1 - MUTATE_RATE)) {
        // Crossover
        // Randomly select two in the TOP_NUM
        int rand_a = rand() % TOP_NUM;
        int rand_b = rand() % TOP_NUM;
        std::vector<ConfigState> individual_a = population[rand_a];
        std::vector<ConfigState> individual_b = population[rand_b];
        int crossover_position = rand() % (SNG_NUM * 3);
        for (int j = 0; j < (SNG_NUM * 3); j++) {
          if (j >= crossover_position) {
            switch (j % 3) {
            case 0: {
              std::swap(individual_a[j / 3].poly_index,
                        individual_b[j / 3].poly_index);
            }
            case 1: {
              std::swap(individual_a[j / 3].seed_index,
                        individual_b[j / 3].seed_index);
            }
            case 2: {
              std::swap(individual_a[j / 3].scram_index,
                        individual_b[j / 3].scram_index);
            }
            }
          }
        }
        // Add to population
        population[i] = individual_a;
        i++;
        if (i < INDIVIDUAL_COUNT)
          population[i] = individual_b;
      } else {
        // Mutation
        int rand_a = rand() % TOP_NUM;
        std::vector<ConfigState> individual = population[rand_a];
        if ((double) rand() / RAND_MAX < 0.2) {
          for (int j = 0; j < individual.size(); j++) {
            std::swap(individual[j], individual[individual.size() - 1 - j]);
          }
        } else {
          for (int j = 0; j < SNG_NUM; j++) {
            if ((double) rand() / RAND_MAX < MUTATE_RATE_PER_SNG) {
              // Mutate individual SNG setting
              int parameter_index = rand() % 3;
              if (parameter_index == 0) {
                individual[j].poly_index = -1;
              } else if (parameter_index == 1) {
                individual[j].seed_index = -1;
              } else if (parameter_index == 2) {
                individual[j].seed_index = -1;
              }
            }
          }
        }
        // Add to population
        population[i] = individual;
      }
    }
  }
  // std::cout << min_result << std::endl;

  return {min_value, min_result};
}

std::pair<double, std::string> SC_sim::SA2Agent::run(Var *var,
                                                     AgentConfig agent_config) {
  double min_value = 10000;
  std::string min_result = "Not implemented in SA2Agent.";
  double ret_value;
  int no_improve_time = 0;

  std::vector<ConfigState> states(var->graph.SNGs.size());

  var->simulate_reset();

  int number_of_runs = 1;
  // int rand_per_run = conf.rand_times / number_of_runs;
  int rand_per_run = var->conf.rand_times;

  for (int run_index = 0; run_index < number_of_runs; run_index++) {
    double min_value_single_run = 10000;

    std::unordered_map<SNGNode *, int> SNG_map(var->graph.SNGs.size());

    for (int index = 0; index < var->graph.SNGs.size(); index++) {
      const auto &sng = var->graph.SNGs[index];
      if (sng->master.empty()) {

        states[index] = sng->randomize();
      }
      SNG_map[sng] = index;
    }

    for (int index = 0; index < var->graph.SNGs.size(); index++) {
      const auto &sng = var->graph.SNGs[index];
      if (!sng->master.empty()) {

        sng->copyConf(sng->master[0]);
        states[index] = states[SNG_map[sng->master[0]]];
      }
    }

    min_value_single_run = var->simulate_once().first;
    min_value = std::min(min_value, min_value_single_run);
    // std::cout << "Initial value: " << min_value_single_run << std::endl;
    double T0 = 2 * min_value_single_run;

    bool no_improve = false;
    // std::cout << "One run: " << std::endl;
    for (int i = 0; i < rand_per_run;) {
      // double temperature = T0 * (1 - (double)i / rand_per_run);
      // std::cout << "Temp: " << temperature << std::endl;

      // Random select one sng
      int rand_index;
      while (true) {
        rand_index = rand() % states.size();
        if (var->graph.SNGs[rand_index]->master.empty())
          break;
      }

      // std::cout << "select new node" << std::endl;

      int no_improve_time_local = 0;
      double temperature = T0;
      while (no_improve_time_local < 5 && i < rand_per_run) {
        temperature = 0.98 * temperature;

        ConfigState current_state = states[rand_index];
        ConfigState new_state = current_state;
        int change = rand() % 200 - 100;

        int parameter_index = rand() % 3;
        if (parameter_index == 0) {
          new_state.poly_index = current_state.poly_index + change;
        } else if (parameter_index == 1) {
          new_state.seed_index = current_state.seed_index + change;
        } else if (parameter_index == 2) {
          new_state.scram_index = current_state.scram_index + change;
        }

        new_state = var->graph.SNGs[rand_index]->randomize(new_state);

        for (int index = 0; index < var->graph.SNGs.size(); index++) {
          const auto &sng = var->graph.SNGs[index];
          if (!sng->master.empty()) {
            sng->copyConf(sng->master[0]);
            // std::cout << "copy " << SNG_map[sng->another] << " to " << index
            //           << std::endl;
          }
        }

        ret_value = var->simulate_once().first;
        i++;

        if (min_value_single_run > ret_value) {
          min_value_single_run = ret_value;
          // std::cout << "Find smaller value: " << min_value_single_run
          //           << std::endl;
          states[rand_index] = new_state;
          for (int index = 0; index < var->graph.SNGs.size(); index++) {
            const auto &sng = var->graph.SNGs[index];

            if (!sng->master.empty()) {

              states[index] = states[SNG_map[sng->master[0]]];
            }
          }
          // min_value = std::min(min_value, min_value_single_run);
          no_improve_time_local = 0;
        } else {
          // Accept the result with probability
          double rand_temp = (double) rand() / RAND_MAX;
          // std::cout << "Rand: " << rand_temp << std::endl;
          // std::cout << "Compare value: "
          //           << std::exp(-(ret_value - min_value_single_run) /
          //                       temperature)
          //           << std::endl;
          if (rand_temp <
              std::exp(-(ret_value - min_value_single_run) / temperature)) {
            min_value_single_run = ret_value;
            // std::cout << "Probalisticly select value: "
            //           << min_value_single_run << std::endl;
            states[rand_index] = new_state;
            for (int index = 0; index < var->graph.SNGs.size(); index++) {
              const auto &sng = var->graph.SNGs[index];

              if (!sng->master.empty()) {

                states[index] = states[SNG_map[sng->master[0]]];
              }
            }
          }
          no_improve_time_local++;
        }
        if (min_value > min_value_single_run) {
          min_value = min_value_single_run;
          no_improve_time = 0;
        } else {
          no_improve_time++;
          if (var->conf.rand_till_converge &&
              no_improve_time > agent_config.NO_IMPROVE_BOUND) {
            no_improve = true;
            break;
          }
        }
        // std::cout << min_value << " " << ret_value << std::endl;
      }
      if (no_improve)
        break;
    }
    if (no_improve)
      break;
  }
  return {min_value, min_result};
}

std::pair<double, std::string>
SC_sim::MCTSAgent::run(Var *var, AgentConfig agent_config) {
  int length = var->graph.SNGs.size();

  class MCTS_State {
  public:
    int step;
    std::vector<ConfigState> actions;
    int max_step;

    MCTS_State(int _step, std::vector<ConfigState> _actions, int _max_step) {
      this->step = _step;
      this->actions = _actions;
      this->max_step = _max_step;
    }

    void set_max_step(int length) { max_step = length; }

    bool is_game_over() { return actions.size() == max_step; }

    MCTS_State move(ConfigState action) {
      std::vector<ConfigState> new_actions = this->actions;
      new_actions.push_back(action);
      return MCTS_State(this->step + 1, new_actions, this->max_step);
    }

    double game_result() {
      double reward = 1;
      assert(0);
      return reward;
    }
  };
  class MCTS_Node {
  public:
    MCTS_State state;
    Node *parent;
    ConfigState parent_action;
    std::vector<MCTS_Node> children;
    int number_of_visits;
    double results;

    double q() { return results; }
    int n() { return number_of_visits; }
    MCTS_Node expand() {
      MCTS_State next_state = this->state.move(ConfigState());
    }
  };
  assert(0 && "Implementing");
  std::cout << "Implementing" << std::endl;
}

std::pair<double, std::string>
SC_sim::DescentAgent::run(Var *var, AgentConfig agent_config) {
  double min_value = 10000;
  std::string min_result = "Not implemented in DescentAgent.";
  double ret_value;
  int no_improve_time = 0;

  std::vector<ConfigState> states(var->graph.SNGs.size());

  var->simulate_reset();

  std::unordered_map<SNGNode *, int> SNG_map(var->graph.SNGs.size());

  for (int index = 0; index < var->graph.SNGs.size(); index++) {
    const auto &sng = var->graph.SNGs[index];
    if (sng->master.empty()) {

      states[index] = sng->randomize();
    }
    SNG_map[sng] = index;
  }

  for (int index = 0; index < var->graph.SNGs.size(); index++) {
    const auto &sng = var->graph.SNGs[index];
    if (!sng->master.empty()) {

      sng->copyConf(sng->master[0]);
      states[index] = states[SNG_map[sng->master[0]]];
    }
  }
  min_value = var->simulate_once().first;

  for (int i = 0; i < var->conf.rand_times; ++i) {
    // Random select one sng
    int rand_index;
    while (true) {
      rand_index = rand() % states.size();
      if (var->graph.SNGs[rand_index]->master.empty())
        break;
    }

    // std::cout << rand_index << std::endl;

    ConfigState new_state = var->graph.SNGs[rand_index]->randomize();

    for (int index = 0; index < var->graph.SNGs.size(); index++) {
      const auto &sng = var->graph.SNGs[index];
      if (!sng->master.empty()) {
        sng->copyConf(sng->master[0]);
        // std::cout << "copy " << SNG_map[sng->another] << " to " << index
        //           << std::endl;
      }
    }

    ret_value = var->simulate_once().first;

    if (min_value > ret_value) {
      min_value = ret_value;
      no_improve_time = 0;

      // Update states
      states[rand_index] = new_state;
      for (int index = 0; index < var->graph.SNGs.size(); index++) {
        const auto &sng = var->graph.SNGs[index];

        if (!sng->master.empty()) {

          states[index] = states[SNG_map[sng->master[0]]];
        }
      }
    } else {
      no_improve_time++;
      if (var->conf.rand_till_converge &&
          no_improve_time > agent_config.NO_IMPROVE_BOUND) {
        break;
      }
    }
    std::cout << min_value << " " << ret_value << std::endl;
  }
  return {min_value, min_result};
}