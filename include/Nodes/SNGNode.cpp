#include "SNGNode.h"
#include "Modules/Sobol.h"
#include "Nodes/Node.h"
#include "Utils/Utils_IO.h"
#include <fstream>
#include <string>

ConfigState SobolNode::copyConf(SNGNode *not_used) {
  assert(!this->master.empty());
  auto *sobolNode = dynamic_cast<SobolNode *>(this->master[0]);
  configured = sobolNode->configured;
  seeds = sobolNode->seeds;
  seed = sobolNode->seed;
  scramblings = sobolNode->scramblings;
  scrambling = sobolNode->scrambling;
  if (this->master.size() == 2) {
    // Xor
    auto *sobolNode_ = dynamic_cast<SobolNode *>(this->master[1]);
    sequence = {};
    assert(sobolNode->sequence.size() == sobolNode_->sequence.size());
    for (int i = 0; i < sobolNode->sequence.size(); i++) {
      sequence.emplace_back(sobolNode->sequence[i] ^ sobolNode_->sequence[i]);
    }
  } else {
    sequence = sobolNode->sequence;
  }
  // TODO! Consider optimization
  lookup_table = sobolNode->lookup_table;

  return ConfigState();
}

std::string SobolNode::get_config_str() {
  std::stringstream str;
  str << "node id: " << this->id << ", ";

  if (!this->master.empty()) {
    str << "sharing node " << this->master[0]->id;
    return str.str();
  }

  str << "seed: " << array_str(this->seed) << ", ";
  str << "scrambling: " << array_str(this->scrambling) << ", ";
  str << "direction vector: ";
  std::vector<int> DV =
      this->sobol.get_DV(Sobol::setting(this->seed, {}, this->scrambling));
  str << array_str(DV);
  return str.str();
}
void SobolNode::config_by_str(const std::string &str) {
  std::string::size_type start;
  std::string::size_type end;
  // Node id
  start = str.find(':');
  end = str.find(',');
  this->id = std::stoi(str.substr(start + 1, end));

  start = str.find('[', end + 1);
  if (start == std::string::npos) {
    // Shared Sobol
    start = str.find("node", end + 1);
    int master_id_temp = std::stoi(str.substr(start + 5));
    std::cout << master_id_temp << std::endl;
    master_id = std::vector<int>(1, master_id_temp);
    return;
  }
  end = str.find(']', start + 1);
  this->seed = parse_str_array(str.substr(start, end + 1));

  start = str.find('[', end + 1);
  end = str.find(']', start + 1);
  this->scrambling = parse_str_array(str.substr(start, end + 1));

  // Not store Direction Vector. It is currently computed from seed.
};

void LFSRNode::config_by_str(const std::string &str) {
  std::string::size_type start;
  std::string::size_type end;
  // Node id
  start = str.find(':');
  end = str.find(',');
  this->id = std::stoi(str.substr(start + 1, end));

  start = str.find('[', end + 1);
  if (start == std::string::npos) {
    // Shared LFSR
    start = str.find("node", end + 1);
    int master_id_temp = std::stoi(str.substr(start + 5));
    std::cout << master_id_temp << std::endl;
    master_id = std::vector<int>(1, master_id_temp);
    return;
  }
  end = str.find(']', start + 1);
  this->polynomial = parse_str_array(str.substr(start, end + 1));
  // print_vector(std::cout, this->polynomial);

  start = str.find('[', end + 1);
  end = str.find(']', start + 1);
  this->seed = parse_str_array(str.substr(start, end + 1));
  // print_vector(std::cout, this->seed);

  start = str.find('[', end + 1);
  end = str.find(']', start + 1);
  this->scrambling = parse_str_array(str.substr(start, end + 1));
  // print_vector(std::cout, this->scrambling);

  LFSR::setting set(seed, polynomial, scrambling, {}, inserting_zero, 0);
  sequence = lfsr.simulate(set);

  if (conf.optimize & INCREMENTAL) {
    lookup_table.resize(sequence.size());
    for (size_t i = 0; i < sequence.size(); i++) {
      lookup_table[sequence[i]] = i;
    }
    if (!inserting_zero)
      lookup_table[0] = -1;
  }
}

std::string SobolNode::verilog_module_init_str() {
  std::stringstream str;
  // Intermediate wire
  str << "\n";
  str << "\twire [bit_width-1:0] wire_Sobol" << this->id << "_cmp" << this->id
      << ";\n";

  switch (this->master.size()) {
  case 0: {
    // Sobol module
    str << "\tSobol" << this->id << " sobol" << this->id << " (\n";
    str << "\t\t.clk(clk),\n";
    str << "\t\t.rst(rst),\n";
    str << "\t\t.Sobol_Data(wire_" << this->label() << this->id << "_cmp"
        << this->id << ")\n";
    str << "\t);\n";
    break;
  }

  case 1: {
    // Connected the output wire from its master to the input of Comparator
    str << "\tassign "
        << "wire_" << this->label() << std::to_string(this->id) << "_cmp"
        << std::to_string(this->id) << " = "
        << "wire_" << this->master[0]->label() << this->master[0]->id << "_cmp"
        << this->master[0]->id << ";\n";
  }
  }
  // Cmp module
  std::string input_str = "\t\t" + verilog_wire_name(this->inputs[0], this);
  std::string output_str = "\t\t" + verilog_wire_name(this, this->outputs[0]);
  str << "\tComparator #(.bit_width(bit_width)) Comparator_" << this->id
      << "(\n"
      << "\t\twire_" << this->label() << this->id << "_cmp" << this->id << ",\n"
      << input_str << ",\n"
      << output_str << "\n"
      << "\t);\n";
  return str.str();
}
std::string
SobolNode::verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
#if 0
  std::ofstream sobol_file("../Verilog/" + conf.graph_name + "/Sobol" +
                           std::to_string(this->id) + ".v");
  sobol_file << "module Sobol" << std::to_string(this->id) << "(\n"
             << "\tclk,\n"
             << "\trst,\n";
  for (int i = 0; i < conf.bit_width; i++) {
    sobol_file << "\tDVMem" << i << ",\n";
  }
  sobol_file << "\tSobol_Data\n"
             << ");\n";

  sobol_file << "\tparameter width = " << conf.bit_width << ";\n"
             << "\n";

  sobol_file << "\tinput rst;\n"
             << "\tinput clk;\n";
  for (int i = 0; i < conf.bit_width; i++) {
    sobol_file << "\tinput [width-1:0] DVMem" << i << ";\n";
  }
  sobol_file << "\n"

             << "\toutput [width-1:0] Sobol_Data;\n";
  sobol_file << "\twire [width-1:0] Direction_Number;\n"
             << "\treg [width-1:0] DVMem_reg[width-1:0];\n"
             << "\treg [width-1:0] Counter_data;\n"
             << "\treg [width-1:0] Sobol_Data;\n"
             << "\n";

  // Impolement Least Significant Zero detection
  sobol_file << "\tassign Direction_Number = (Counter_data[0] == 1'b0) ? "
                "DVMem_reg[0] :\n";
  for (int i = 1; i < conf.bit_width; i++) {
    sobol_file << "\t\t(Counter_data[" << i << "] == 1'b0) ? DVMem_reg[" << i
               << "] :\n";
  }
  sobol_file << "\t\t0;\n"
             << "\n";

  // Always block
  sobol_file << "\talways @(posedge clk) begin\n"
                "\t\tif (rst) begin\n"
                "\t\t\tSobol_Data   = 0;\n"
                "\t\t\tCounter_data = 0;\n";
  for (int i = 0; i < conf.bit_width; i++) {
    sobol_file << "\t\t\tDVMem_reg[" << i << "] = DVMem" << i << ";\n";
  }
  sobol_file << "\t\tend else begin\n"
             << "\t\t\tSobol_Data   = Sobol_Data ^ Direction_Number;\n"
             << "\t\t\tCounter_data = Counter_data + 1;\n"
             << "\t\tend\n"
             << "\tend\n"
             << "\n";

  sobol_file << "endmodule";
#endif
  // Initial Direction Vectors in the module
#if 1
  if (this->master.empty()) {
    std::ofstream sobol_file("../Verilog/" + conf.graph_name + "/Sobol" +
                             std::to_string(this->id) + ".v");
    sobol_file << "module Sobol" << std::to_string(this->id) << "(\n"
               << "\tclk,\n"
               << "\trst,\n";
    sobol_file << "\tSobol_Data\n"
               << ");\n";

    sobol_file << "\tparameter width = " << conf.bit_width << ";\n"
               << "\n";

    sobol_file << "\tinput rst;\n"
               << "\tinput clk;\n";
    sobol_file << "\n"
               << "\toutput [width-1:0] Sobol_Data;\n"
               << "\n";

    sobol_file << "\twire [width-1:0] Direction_Number;\n"
               << "\treg [width-1:0] DVMem_reg[width-1:0];\n"
               << "\treg [width-1:0] Counter_data;\n"
               << "\treg [width-1:0] Sobol_Data_reg;\n"
               << "\n";

    // Implement Least Significant Zero detection
    sobol_file << "\t// Least Significant Zero detection\n"
                  "\tassign Direction_Number = \n";
    for (int i = 0; i < conf.bit_width; i++) {
      sobol_file << "\t\t(Counter_data[" << i << "] == 1'b0) ? DVMem_reg[" << i
                 << "] :\n";
    }
    sobol_file << "\t\t0;\n"
               << "\n";

    // Implement scrambling
    sobol_file << "\t// Scrambling\n";
    for (size_t tmp_i = 0; tmp_i < this->scrambling.size(); tmp_i++) {
      sobol_file << "\tassign Sobol_Data[width-1-" << tmp_i
                 << "] = Sobol_Data_reg[width-1-"
                 << std::to_string(this->scrambling[tmp_i]) << "];\n";
    }
    sobol_file << "\n";

    // Always block
    sobol_file << "\talways @(posedge clk) begin\n"
                  "\t\tif (rst) begin\n"
                  "\t\t\tSobol_Data_reg = 0;\n"
                  "\t\t\tCounter_data   = 0;\n";
    // Initialize Direction Vector in the always block when reset
    Sobol::setting set(seed);
    std::vector<int> Dv = this->sobol.get_DV(set);
    for (int i = 0; i < conf.bit_width; i++) {
      sobol_file << "\t\t\tDVMem_reg[" << i << "]   = " << Dv[i] << ";\n";
    }
    sobol_file << "\t\tend else begin\n"
               << "\t\t\tSobol_Data_reg = Sobol_Data_reg ^ Direction_Number;\n"
               << "\t\t\tCounter_data   = Counter_data + 1;\n"
               << "\t\tend\n"
               << "\tend\n"
               << "\n";

    sobol_file << "endmodule";
  } else {
    // Do nothing, as this is a shared Sobol
  }

  // Generate Comparator.v
  // If exist, don't generate
  std::string cmp_file_path = "../Verilog/" + conf.graph_name + "/Comparator.v";
  bool cmp_file_exist = std::ifstream(cmp_file_path.c_str()).good();
  if (!cmp_file_exist) {
    std::ofstream cmp_file(cmp_file_path);
    cmp_file << "module Comparator (\n"
                "\ti1,\n"
                "\ti2,\n"
                "\to1\n"
                ");\n"
             << "\tparameter bit_width = " << conf.bit_width << ";\n"
             << "\n";

    cmp_file << "\tinput [bit_width-1:0] i1;\n"
                "\tinput [bit_width-1:0] i2;\n"
                "\toutput o1;\n"
             << "\n";

    if (conf.scType == unipolar) {
      cmp_file << "\tassign o1 = i1 < i2;\n";
    } else {
      // Bipolar encoding
      cmp_file << "\tassign o1 = $signed(i1) < $signed(i2);\n";
    }

    cmp_file << "\n";

    cmp_file << "endmodule\n";
  }

#endif
  return "";
}

std::string SobolNode::get_config() {
  std::stringstream str;
  str << "leq: ";
  str << leq;
  str << " inserting_zero: ";
  str << inserting_zero;
  return str.str();
}

SobolNode::SobolNode(int _input_num, int _output_num, const Config &_conf,
                     std::vector<SNGNode *> _master)
    : SNGNode(_input_num, _output_num, _conf, _master), sobol(conf.bit_width) {
  N = conf.bit_width;
  leq = conf.leq;
  inserting_zero = conf.inserting_zero;

  // polynomials = sobol.search_polynomials();
  // seeds = generate_seeding(N);
  scramblings = generate_scrambling(N);

  configured = true;
};
Node *SobolNode::copy() {
  SobolNode *new_node = new SobolNode(this->input_num, this->output_num,
                                      this->conf, this->master);
  new_node->init(this->extra);
  new_node->configure(this->leq, this->inserting_zero);
  new_node->id = this->id;
  return new_node;
}
void SobolNode::configure(bool _leq, bool _inserting_zero) {
  leq = _leq;
  inserting_zero = _inserting_zero;

  configured = true;
}

void SobolNode::run() {
  if (conf.optimize & INCREMENTAL) {
    update.clear();
    int x = inputs[0]->data[0];
    if (prev == x) {
      // Do nothing
    } else {
      if (prev == x - 1) {
        if (leq) {
          data[lookup_table[x]] = 1;
          update.insert(lookup_table[x]);
        } else {
          data[lookup_table[x - 1]] = 1;
          update.insert(lookup_table[x - 1]);
        }
        prev = x;
      } else {
        bitstream_list new_data;
        if (conf.scType == unipolar)
          new_data = comparator(sequence, x, leq);
        else
          new_data = comparator(sequence, x, leq, true, conf.bit_width);
        for (size_t i = 0; i < data.size(); i++) {
          if (new_data[i] != data[i]) {
            update.insert(i);
            data[i] = new_data[i];
          }
        }
        prev = x;
      }
    }
  } else {   // LOOKUP_TABLE or RUNNING_MINIMAL or None optimization
    int x = inputs[0]->data[0];
    if (conf.scType == unipolar)
      data = comparator(sequence, x, leq);
    else
      data = comparator(sequence, x, leq, true, conf.bit_width);
  }
  if (conf.optimize & LOOKUPTABLE) {
    if (!(conf.optimize & INCREMENTAL)) {
      int x = inputs[0]->data[0];
      if (conf.scType == unipolar)
        data = comparator(sequence, x, leq);
      else
        data = comparator(sequence, x, leq, true, conf.bit_width);
    }
  }
#if LOG
  std::cout << "SobolNode: ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif

  ready = true;
  notify();
}

void SobolNode::reset_zero() {
  update.clear();
  prev = inputs[0]->data[0];
  if (conf.scType == unipolar)
    data = comparator(sequence, prev, leq);
  else
    data = comparator(sequence, prev, leq, true, conf.bit_width);

#if LOG
  std::cout << "LFSRNode reset_zero(): ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif

  ready = true;
  notify_reset_zero();
}
ConfigState SobolNode::randomize(ConfigState state = ConfigState()) {

  seed = sobol.random_seed();
  // polynomial = polynomials[rand() % polynomials.size()];
  scrambling = scramblings[rand() % scramblings.size()];
  Sobol::setting set(seed, {}, scrambling);
  sequence = sobol.simulate(set);

  if (conf.optimize & INCREMENTAL) {
    lookup_table.resize(sequence.size());
    for (size_t i = 0; i < sequence.size(); i++) {
      lookup_table[sequence[i]] = i;
    }
  }

  return {};
}

std::string LFSRNode::verilog_module_init_str() {
  // lfsr
  std::stringstream str;
  // wire
  str << "\twire [bit_width-1:0] wire_" << this->label()
      << std::to_string(this->id) << "_cmp" << std::to_string(this->id)
      << ";\n";

  std::string lfsr_declaration = "";

  switch (this->master.size()) {
  case 0: {
    // independent lfsr
    std::string seed_str = std::to_string(seed.size()) + "\'b";
    for (size_t tmp_i = 0; tmp_i < this->seed.size(); tmp_i++) {
      seed_str += std::to_string(seed[tmp_i]);
    }

    str << "\t" << label() << this->id << " #(.bit_width(bit_width)) "
        << label() << this->id << "(\n"
        << "\t\t.clk(clk),\n"
        << "\t\t.rst(rst),\n"
        << "\t\t.seed(" << seed_str << "),\n"
        << "\t\t.LFSR_Data(wire_" << this->label() << this->id << "_cmp"
        << this->id << ")\n"
        << "\t);\n";
    // comparator
    std::string input_str, output_str;
    for (auto &i : inputs) {
      if (i->verilog_has_instance())   // ensure Node such as CalculateNode is
                                       // not considered
        input_str += "\t\t" + verilog_wire_name(i, this) + ",\n";
    }
    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (i->verilog_has_instance()) {
        output_str += "\t\t" + verilog_wire_name(this, i) + "\n";
        break;
      }
    }
    str << "\tComparator #(.bit_width(bit_width)) Comparator_" << this->id
        << "(\n"
        << "\t\twire_" << this->label() << this->id << "_cmp" << this->id
        << ",\n"
        << input_str << output_str << "\t);\n";
    break;
  }
  case 1: {
    // Connected the output wire from its master to the input of Comparator
    str << "\tassign "
        << "wire_" << this->label() << std::to_string(this->id) << "_cmp"
        << std::to_string(this->id) << " = "
        << "wire_LFSR" << this->master[0]->id << "_cmp" << this->master[0]->id
        << ";\n";

    // Comparator
    std::string input_str, output_str;
    for (auto &i : inputs) {
      if (i->verilog_has_instance())   // ensure Node such as CalculateNode is
                                       // not considered
        input_str += "\t\t" + verilog_wire_name(i, this) + ",\n";
    }
    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (i->verilog_has_instance()) {
        output_str += "\t\t" + verilog_wire_name(this, i) + "\n";
        break;
      }
    }
    str << "\tComparator #(.bit_width(bit_width)) Comparator_" << this->id
        << "(\n"
        << "\t\twire_" << this->label() << this->id << "_cmp" << this->id
        << ",\n"
        << input_str << output_str << "\t);\n";
    break;
  }
  case 2: {
    assert(0 && "not checked.");
    // xor from two master lfsr
    lfsr_declaration += "\tXor #(.bit_width(bit_width),.input_num(2)) Xor_" +
                        std::to_string(this->id) + "(\n" + "\t\t{wire_" +
                        std::to_string(this->master[0]->id) + "_lfsr_comp," +
                        "\t\twire_" + std::to_string(this->master[1]->id) +
                        "_lfsr_comp},\n" + "\t\twire_" +
                        std::to_string(this->id) + "_xor_comp);\n";
    // comparator
    std::string input_str, output_str;
    for (auto &i : inputs) {
      if (i->verilog_has_instance())   // ensure Node such as CalculateNode is
                                       // not considered
        input_str += "\t\twire_" + std::to_string(i->id) + "_" +
                     std::to_string(this->id) + ",\n";
    }
    for (auto &i : outputs) {
      // definition of Verilog Module only contain one output
      // the connection of this output to different node is handled in
      // verilog_codegen_multi_output
      if (i->verilog_has_instance()) {
        output_str += "\t\twire_" + std::to_string(this->id) + "_" +
                      std::to_string(i->id);
        break;
      }
    }
    lfsr_declaration += "\tComparator #(.bit_width(bit_width),.is_signed(" +
                        std::to_string(conf.scType == bipolar) +
                        ")) Comparator_" + std::to_string(this->id) + "(\n" +
                        input_str + "\t\twire_" + std::to_string(this->id) +
                        "_xor_comp,\n" + output_str + ");\n";
    break;
  }
  default:
    assert(1);
  }
  str << lfsr_declaration;
  return str.str();
}

void LFSRNode::reset_zero() {
  update.clear();
  prev = inputs[0]->data[0];
  if (conf.scType == unipolar)
    data = comparator(sequence, prev, leq);
  else
    data = comparator(sequence, prev, leq, true, conf.bit_width);

#if LOG
  std::cout << "LFSRNode reset_zero(): ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif

  ready = true;
  notify_reset_zero();
}
ConfigState LFSRNode::randomize(ConfigState state = ConfigState()) {
  if (!configured) {
    std::cerr << "(info): LFSR has not been configured, use default "
                 "configurations."
              << std::endl;
  }
  if (!fixed_config.contains("seed")) {
    if (state.rand || state.seed_index < 0 ||
        state.seed_index > seeds.size() - 1) {
      state.seed_index = rand() % seeds.size();
    }
    seed = seeds[state.seed_index];
  }
  if (!fixed_config.contains("polynomial")) {
    if (state.rand || state.poly_index < 0 ||
        state.poly_index > polynomials.size() - 1) {
      state.poly_index = rand() % polynomials.size();
    }
    polynomial = polynomials[state.poly_index];
  }
  if (!fixed_config.contains("scrambling")) {
    if (state.rand || state.scram_index < 0 ||
        state.scram_index > polynomials.size() - 1) {
      state.scram_index = rand() % scramblings.size();
    }
    scrambling = scramblings[state.scram_index];
  }
  LFSR::setting set(seed, polynomial, scrambling, {}, inserting_zero, 0);
  sequence = lfsr.simulate(set);

  if (conf.optimize & INCREMENTAL) {
    lookup_table.resize(sequence.size());
    for (size_t i = 0; i < sequence.size(); i++) {
      lookup_table[sequence[i]] = i;
    }
    if (!inserting_zero)
      lookup_table[0] = -1;
  }

  state.rand = false;
  return state;
}
ConfigState LFSRNode::copyConf(SNGNode *not_used) {
  assert(!this->master.empty());
  auto *lfsrNode = dynamic_cast<LFSRNode *>(this->master[0]);
  configured = lfsrNode->configured;
  seeds = lfsrNode->seeds;
  seed = lfsrNode->seed;
  polynomials = lfsrNode->polynomials;
  polynomial = lfsrNode->polynomial;
  scramblings = lfsrNode->scramblings;
  scrambling = lfsrNode->scrambling;
  if (this->master.size() == 2) {
    // Xor
    auto *lfsrNode_ = dynamic_cast<LFSRNode *>(this->master[1]);
    sequence = {};
    assert(lfsrNode->sequence.size() == lfsrNode_->sequence.size());
    for (int i = 0; i < lfsrNode->sequence.size(); i++) {
      sequence.emplace_back(lfsrNode->sequence[i] ^ lfsrNode_->sequence[i]);
    }
  } else {
    sequence = lfsrNode->sequence;
  }
  // TODO! Consider optimization
  lookup_table = lfsrNode->lookup_table;

  return ConfigState();
}
void LFSRNode::configure(bool _leq, bool _inserting_zero) {
  leq = _leq;
  inserting_zero = _inserting_zero;

  configured = true;
}

std::string LFSRNode::get_config_str() {

  std::stringstream config_str;

  config_str << "node id: " << id << ", ";

  if (!this->master.empty()) {
    config_str << "sharing node " << this->master[0]->id;
    return config_str.str();
  }

  config_str << "polynomial: [";
  for (size_t i = 0; i < polynomial.size() - 1; i++) {
    config_str << polynomial[i] << ", ";
  }
  config_str << polynomial[polynomial.size() - 1] << "], ";

  config_str << "seed: [";
  for (size_t i = 0; i < seed.size() - 1; i++) {
    config_str << seed[i] << ", ";
  }
  config_str << seed[seed.size() - 1] << "], ";

  config_str << "scrambling: [";
  for (size_t i = 0; i < scrambling.size() - 1; i++) {
    config_str << scrambling[i] << ", ";
  }
  config_str << scrambling[scrambling.size() - 1] << "], ";

  config_str << get_config() << "\n";
  return config_str.str();
}
std::string
LFSRNode::verilog_codegen_str(std::unordered_map<Node *, int> node_index_map) {
  // not copy or xor from other lfsr
  if (this->master.empty()) {
    // generate a LFSR
    std::ofstream lfsr_file("../Verilog/" + conf.graph_name + "/LFSR" +
                            std::to_string(this->id) + ".v");
    lfsr_file << "module LFSR" << std::to_string(this->id) << " (\n"
              << "\tclk,\n"
              << "\trst,\n"
              << "\tseed,\n"
              << "\tLFSR_Data\n"
              << ");\n";

    lfsr_file << "\tparameter bit_width = " << conf.bit_width << ";\n"
              << "\n";

    lfsr_file << "\tinput clk;\n"
                 "\tinput rst;\n"
                 "\tinput [bit_width-1:0] seed; //Initial value\n"
                 "\toutput [bit_width-1:0] LFSR_Data; // output \n"
                 "\n";

    lfsr_file << "\treg [bit_width-1:0] shift_reg;\n"
              << "\n";

    // scrambling
    std::string scram;
    for (size_t tmp_i = 0; tmp_i < this->scrambling.size(); tmp_i++) {
      scram += "\tassign LFSR_Data[bit_width-1-" + std::to_string(tmp_i) +
               "] = shift_reg[bit_width-1-" +
               std::to_string(this->scrambling[tmp_i]) + "];\n";
    }
    lfsr_file << scram << "\n";

    // tap
    lfsr_file << "\talways @(posedge clk) begin\n"
                 "\t\tif (rst) shift_reg = seed;\n"
                 "\t\telse\n"
                 "\t\t\tshift_reg = { ";
    std::string taps = "shift_reg[0] ^ ";
    for (size_t tmp_i = 0; tmp_i < this->polynomial.size() - 1; tmp_i++) {
      taps += "shift_reg[bit_width-1-" +
              std::to_string(this->polynomial[tmp_i]) + "] ^ ";
    }
    taps += "shift_reg[bit_width-1-" +
            std::to_string(polynomial[polynomial.size() - 1]) + "] ";
    lfsr_file << taps
              << ",shift_reg[bit_width-1:1]};\n"
                 "\tend\n"
                 "\n";

    lfsr_file << "endmodule\n" << std::endl;
  } else {
    // Do nothing, as this is a shared LFSR
  }

  // Generate Comparator.v
  // If exist, don't generate
  std::string cmp_file_path = "../Verilog/" + conf.graph_name + "/Comparator.v";
  bool cmp_file_exist = std::ifstream(cmp_file_path.c_str()).good();
  if (!cmp_file_exist) {
    std::ofstream cmp_file(cmp_file_path);
    cmp_file << "module Comparator (\n"
                "\ti1,\n"
                "\ti2,\n"
                "\to1\n"
                ");\n"
             << "\tparameter bit_width = " << conf.bit_width << ";\n"
             << "\n";

    cmp_file << "\tinput [bit_width-1:0] i1;\n"
                "\tinput [bit_width-1:0] i2;\n"
                "\toutput o1;\n"
             << "\n";

    if (conf.scType == unipolar) {
      cmp_file << "\tassign o1 = i1 < i2;\n";
    } else {
      // Bipolar encoding
      cmp_file << "\tassign o1 = $signed(i1) < $signed(i2);\n";
    }

    cmp_file << "\n";

    cmp_file << "endmodule\n";
  }

  return "";
}
std::string LFSRNode::get_config() {
  std::stringstream str;
  str << "leq: ";
  str << leq;
  str << " inserting_zero: ";
  str << inserting_zero;
  return str.str();
}
std::string LFSRNode::codegen_str() {
  return "    LFSRNode *<-name-> = new LFSRNode(" + std::to_string(input_num) +
         ", " + std::to_string(output_num) + ", conf);" +
         //  "\n    <-name->->configure(conf.leq, conf.inserting_zero);" +
         "\n    <-name->->init(<-name->_extra);" +
         "\n    <-name->->randomize();" + "\n    SNGs.push_back(<-name->);";
}
Node *LFSRNode::copy() {
  LFSRNode *new_node =
      new LFSRNode(this->input_num, this->output_num, this->conf, this->master);
  new_node->init(this->extra);
  new_node->configure(this->leq, this->inserting_zero);
  new_node->id = this->id;
  return new_node;
}

void LFSRNode::init(const std::vector<std::string> &_extra) {
  extra = _extra;
  parse_extra();
}

LFSRNode::LFSRNode(int _input_num, int _output_num, const Config &_conf,
                   std::vector<SNGNode *> _master)
    : SNGNode(_input_num, _output_num, _conf, _master), lfsr(conf.bit_width) {
  N = conf.bit_width;
  leq = conf.leq;
  inserting_zero = conf.inserting_zero;

  polynomials = lfsr.search_polynomials();
  seeds = generate_seeding(N);
  scramblings = generate_scrambling(N);

  configured = true;
};
void LFSRNode::parse_extra() {
  if (extra.empty())
    return;

  size_t index = 0;
  if (extra[index] == "True") {
    while (index < extra.size()) {
      if (extra[index] == "end") {
        index++;
        break;
      }
      index++;
    }
  }
  while (index < extra.size()) {
    if (extra[index] == "seed:") {
      index++;
      while (extra[index] != "]") {
        seed.push_back(std::stoi(extra[index + 1]));
        index += 2;
      }
      index++;
      fixed_config.insert("seed");
    } else if (extra[index] == "scrambling:") {
      index++;
      while (extra[index] != "]") {
        scrambling.push_back(std::stoi(extra[index + 1]));
        index += 2;
      }
      index++;
      fixed_config.insert("scrambling");
    } else if (extra[index] == "polynomial:") {
      index++;
      while (extra[index] != "]") {
        polynomial.push_back(std::stoi(extra[index + 1]));
        index += 2;
      }
      index++;
      fixed_config.insert("polynomial");
    } else {
      assert(0 && "Unknow config for LFSR");
    }
    index++;
  }
}
void LFSRNode::run() {
  if (conf.optimize & INCREMENTAL) {
    update.clear();
    int x = inputs[0]->data[0];
    if (prev == x) {
      // Do nothing
    } else {
      if (prev == x - 1) {
        if (leq) {
          data[lookup_table[x]] = 1;
          update.insert(lookup_table[x]);
        } else {
          if (x == 1 && !inserting_zero) {
          } else {
            data[lookup_table[x - 1]] = 1;
            update.insert(lookup_table[x - 1]);
          }
        }
        prev = x;
      } else {
        bitstream_list new_data;
        if (conf.scType == unipolar)
          new_data = comparator(sequence, x, leq);
        else
          new_data = comparator(sequence, x, leq, true, conf.bit_width);
        for (size_t i = 0; i < data.size(); i++) {
          if (new_data[i] != data[i]) {
            update.insert(i);
            data[i] = new_data[i];
          }
        }
        prev = x;
      }
    }
  } else {   // LOOKUP_TABLE or RUNNING_MINIMAL or None optimization
    int x = inputs[0]->data[0];
    if (conf.scType == unipolar)
      data = comparator(sequence, x, leq);
    else
      data = comparator(sequence, x, leq, true, conf.bit_width);
  }
  if (conf.optimize & LOOKUPTABLE) {
    if (!(conf.optimize & INCREMENTAL)) {
      int x = inputs[0]->data[0];
      if (conf.scType == unipolar)
        data = comparator(sequence, x, leq);
      else
        data = comparator(sequence, x, leq, true, conf.bit_width);
    }
  }
#if LOG
  std::cout << "LFSRNode: ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif

  ready = true;
  notify();
}

void BinomialNode::run() {
  if (conf.optimize == NONE || conf.optimize == LOOKUPTABLE) {
    int x = inputs[0]->data[0];
    if (conf.scType == unipolar)
      data = comparator(sequence, x, leq);
    else
      data = comparator(sequence, x, leq, true, conf.bit_width);
  } else if (conf.optimize == INCREMENTAL) {
    update.clear();
    int x = inputs[0]->data[0];
    if (prev == x) {
      // Do nothing
    } else {
      assert(prev == x - 1);
      bitstream_list prev_data = data;
      if (conf.scType == unipolar)
        data = comparator(sequence, x, leq);
      else
        data = comparator(sequence, x, leq, true, conf.bit_width);
      for (size_t i = 0; i < data.size(); i++) {
        if (prev_data[i] != data[i])
          update.insert(i);
      }
      prev = x;
    }
  }
#if LOG
  std::cout << "BinomialNode: ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif

  ready = true;
  notify();
}

void BinomialNode::reset_zero() {
  update.clear();
  prev = inputs[0]->data[0];
  if (conf.scType == unipolar)
    data = comparator(sequence, prev, leq);
  else
    data = comparator(sequence, prev, leq, true, conf.bit_width);

#if LOG
  std::cout << "BinomialNode reset_zero(): ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif

  ready = true;
  notify_reset_zero();
}
ConfigState BinomialNode::randomize(ConfigState state = ConfigState()) {
  if (!configured) {
    std::cerr << "(info): Binomial has not been configured, use default "
                 "configurations."
              << std::endl;
  }
  sequence = SNG::simulate(BINOMIAL_t, N, 0, inserting_zero);

  return state;
}
void BinomialNode::configure(bool _leq, bool _inserting_zero) {
  leq = _leq;
  inserting_zero = _inserting_zero;

  configured = true;
}
std::string BinomialNode::get_config() {
  std::stringstream str;
  str << "leq: ";
  str << leq;
  str << " inserting_zero: ";
  str << inserting_zero;
  return str.str();
}
std::string BinomialNode::codegen_str() {
  return "    BinomialNode *<-name-> = new BinomialNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf);" +
         //  "\n    <-name->->configure(conf.leq, conf.inserting_zero);" +
         "\n    <-name->->randomize();";
}
BinomialNode::BinomialNode(int _input_num, int _output_num, const Config &_conf)
    : SNGNode(_input_num, _output_num, _conf) {
  N = conf.bit_width;
  leq = conf.leq;
  inserting_zero = conf.inserting_zero;
  configured = true;
};
void HypergeometricNode::run() {
  if (conf.optimize & INCREMENTAL) {
    update.clear();
    int x = inputs[0]->data[0];
    if (prev == x) {
      // Do nothing
    } else {
      // assert(prev == x - 1);
      if (prev == x - 1) {
        if (leq) {
          data[lookup_table[x]] = 1;
          update.insert(lookup_table[x]);
        } else {
          if (x == 1 && !inserting_zero) {
          } else {
            data[lookup_table[x - 1]] = 1;
            update.insert(lookup_table[x - 1]);
          }
        }
        prev = x;
      } else {
        bitstream_list new_data;
        if (conf.scType == unipolar)
          new_data = comparator(sequence, x, leq);
        else
          new_data = comparator(sequence, x, leq, true, conf.bit_width);

        for (size_t i = 0; i < data.size(); i++) {
          if (new_data[i] != data[i]) {
            update.insert(i);
            data[i] = new_data[i];
          }
        }
        prev = x;
      }
    }
  }

  if (conf.optimize == NONE) {
    int x = inputs[0]->data[0];
    if (conf.scType == unipolar)
      data = comparator(sequence, x, leq);
    else
      data = comparator(sequence, x, leq, true, conf.bit_width);
  }
  if (conf.optimize & LOOKUPTABLE) {
    if (!(conf.optimize & INCREMENTAL)) {
      int x = inputs[0]->data[0];
      if (conf.scType == unipolar)
        data = comparator(sequence, x, leq);
      else
        data = comparator(sequence, x, leq, true, conf.bit_width);
    }
  }
#if LOG
  std::cout << "HypergeometricNode: ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif

  ready = true;
  notify();
}
void HypergeometricNode::reset_zero() {
  update.clear();
  prev = inputs[0]->data[0];
  if (conf.scType == unipolar)
    data = comparator(sequence, prev, leq);
  else
    data = comparator(sequence, prev, leq, true, conf.bit_width);
#if LOG
  std::cout << "HypergeometricNode reset_zero(): ";
  print_vector(std::cout, data);
  std::cout << std::endl;
#endif
  ready = true;
  notify_reset_zero();
}
ConfigState HypergeometricNode::randomize(ConfigState state = ConfigState()) {
  if (!configured) {
    std::cerr << "(info): Hypergeometric has not been configured, use default "
                 "configurations."
              << std::endl;
  }
  sequence = SNG::simulate(HYPERGEOMETRIC_t, N, 0, inserting_zero);

  if (conf.optimize & INCREMENTAL) {
    lookup_table.resize(sequence.size());
    for (size_t i = 0; i < sequence.size(); i++) {
      lookup_table[sequence[i]] = i;
    }
    if (!inserting_zero)
      lookup_table[0] = -1;
  }

  return state;
}
void HypergeometricNode::configure(bool _leq, bool _inserting_zero) {
  leq = _leq;
  inserting_zero = _inserting_zero;

  configured = true;
}
std::string HypergeometricNode::get_config() {
  std::stringstream str;
  str << "leq: ";
  str << leq;
  str << " inserting_zero: ";
  str << inserting_zero;
  return str.str();
}
std::string HypergeometricNode::codegen_str() {
  return "    HypergeometricNode *<-name-> = new HypergeometricNode(" +
         std::to_string(input_num) + ", " + std::to_string(output_num) +
         ", conf);" +
         //  "\n    <-name->->configure(conf.leq, conf.inserting_zero);" +
         "\n    <-name->->randomize();";
}
HypergeometricNode::HypergeometricNode(int _input_num, int _output_num,
                                       const Config &_conf)
    : SNGNode(_input_num, _output_num, _conf) {
  N = conf.bit_width;
  leq = conf.leq;
  inserting_zero = conf.inserting_zero;
  configured = true;
  assert(0);
};