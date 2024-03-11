cmake_minimum_required(VERSION 3.16)
project(Stochastic_computing)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(MY_FLAGS "")
string(JOIN " " MY_FLAGS
  "-Wall"
  "-fdiagnostics-color=always"
  "-g"

  "-O2"
)

set(CMAKE_CXX_FLAGS ${MY_FLAGS})

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(INCLUDE_DIR "include")

set(HEADER_FILES
  ${INCLUDE_DIR}/Headers.h

  ${INCLUDE_DIR}/Utils/Utils.cpp
  ${INCLUDE_DIR}/Utils/Utils.h
  ${INCLUDE_DIR}/Utils/Utils_IO.cpp
  ${INCLUDE_DIR}/Utils/Utils_IO.h

  ${INCLUDE_DIR}/Modules/Gates.cpp
  ${INCLUDE_DIR}/Modules/Gates.h
  ${INCLUDE_DIR}/Modules/LFSR.cpp
  ${INCLUDE_DIR}/Modules/LFSR.h
  ${INCLUDE_DIR}/Modules/Sobol.cpp
  ${INCLUDE_DIR}/Modules/Sobol.h
  ${INCLUDE_DIR}/Modules/SC_core.cpp
  ${INCLUDE_DIR}/Modules/SC_core.h
  ${INCLUDE_DIR}/Modules/SNG.h
  ${INCLUDE_DIR}/Modules/SNG.cpp
  ${INCLUDE_DIR}/Modules/Var.h
  ${INCLUDE_DIR}/Modules/Var.cpp
  ${INCLUDE_DIR}/Modules/Expression.h
  ${INCLUDE_DIR}/Modules/Expression.cpp
  ${INCLUDE_DIR}/Modules/Op.h
  ${INCLUDE_DIR}/Modules/Op.cpp
  ${INCLUDE_DIR}/Modules/DAG.h
  ${INCLUDE_DIR}/Modules/DAG.cpp
  ${INCLUDE_DIR}/Modules/Agent.h
  ${INCLUDE_DIR}/Modules/Agent.cpp

  ${INCLUDE_DIR}/CodeGen/CodeGen.h
  ${INCLUDE_DIR}/CodeGen/CodeGen.cpp

  ${INCLUDE_DIR}/Nodes/CalculateNode.h
  ${INCLUDE_DIR}/Nodes/CalculateNode.cpp
  ${INCLUDE_DIR}/Nodes/FunctionNode.h
  ${INCLUDE_DIR}/Nodes/FunctionNode.cpp
  ${INCLUDE_DIR}/Nodes/Node.cpp
  ${INCLUDE_DIR}/Nodes/Node.h
  ${INCLUDE_DIR}/Nodes/NodeFactory.cpp
  ${INCLUDE_DIR}/Nodes/NodeFactory.h
  ${INCLUDE_DIR}/Nodes/OtherNode.h
  ${INCLUDE_DIR}/Nodes/OtherNode.cpp
  ${INCLUDE_DIR}/Nodes/DisplayNode.h
  ${INCLUDE_DIR}/Nodes/SNGNode.h
  ${INCLUDE_DIR}/Nodes/SNGNode.cpp
  ${INCLUDE_DIR}/Nodes/SourceNode.h
  ${INCLUDE_DIR}/Nodes/SourceNode.cpp
  ${INCLUDE_DIR}/Nodes/OptimizeNode.h
  ${INCLUDE_DIR}/Nodes/BinaryNode.h
  ${INCLUDE_DIR}/Nodes/BinaryNode.cpp
)

function(add_circuit target_name file_name)
  add_executable(${target_name} Circuits_API/${file_name}.cpp ${HEADER_FILES})
  target_include_directories(${target_name} PRIVATE "include/")
  target_link_libraries(${target_name} -lginac -lcln)
  target_link_libraries(${target_name} ${OpenCV_LIBS})
endfunction()

# # Implement your own circuits using SCGen in Circuits_API folder
add_circuit(circuit_api_example circuit_api_example)

# # Acceleration Testbenches
add_circuit(x2 x2)
add_circuit(xy xy)
add_circuit(x_plus_y x_plus_y)
add_circuit(GB_3x3 GB_3x3)
add_circuit(GB_5x5 GB_5x5)
add_circuit(NN_4 NN_4)
add_circuit(NN_8 NN_8)

# # DSE Methods Testbenches (GA and SA and RANDOM)
add_circuit(x2_DSE ../Benchmarks/DSE_methods/x2_DSE)
add_circuit(xy_DSE ../Benchmarks/DSE_methods/xy_DSE)
add_circuit(x_plus_y_DSE ../Benchmarks/DSE_methods/x_plus_y_DSE)
add_circuit(roberts_DSE ../Benchmarks/DSE_methods/roberts_DSE)
add_circuit(sobel_DSE ../Benchmarks/DSE_methods/sobel_DSE)
add_circuit(interpolation_DSE ../Benchmarks/DSE_methods/interpolation_DSE)
add_circuit(GB_3x3_DSE ../Benchmarks/DSE_methods/GB_3x3_DSE)
add_circuit(GB_5x5_DSE ../Benchmarks/DSE_methods/GB_5x5_DSE)
add_circuit(NN_4_DSE ../Benchmarks/DSE_methods/NN_4_DSE)
add_circuit(NN_8_DSE ../Benchmarks/DSE_methods/NN_8_DSE)
add_circuit(x2_scsynth ../Benchmarks/DSE_methods/x2_scsynth)
add_circuit(GB_3x3_scsynth ../Benchmarks/DSE_methods/GB_3x3_scsynth)
add_circuit(x2_BitSAD ../Benchmarks/DSE_methods/x2_BitSAD)
add_circuit(GB_3x3_BitSAD ../Benchmarks/DSE_methods/GB_3x3_BitSAD)

# # Verilog Generation Testbenches
add_circuit(x_plus_y_Gen ../Benchmarks/Verilog_gen/x_plus_y_Gen)
add_circuit(interpolation_Gen ../Benchmarks/Verilog_gen/interpolation_Gen)
add_circuit(GB_3x3_Gen ../Benchmarks/Verilog_gen/GB_3x3_Gen)
add_circuit(GB_5x5_Gen ../Benchmarks/Verilog_gen/GB_5x5_Gen)
add_circuit(NN_4_Gen ../Benchmarks/Verilog_gen/NN_4_Gen)
add_circuit(NN_8_Gen ../Benchmarks/Verilog_gen/NN_8_Gen)

# # RNS Sharing Testbenches
add_circuit(x_plus_y_shared ../Benchmarks/RNS_sharing/x_plus_y_shared)
add_circuit(interpolation_shared ../Benchmarks/RNS_sharing/interpolation_shared)
add_circuit(GB_3x3_shared ../Benchmarks/RNS_sharing/GB_3x3_shared)
add_circuit(GB_5x5_shared ../Benchmarks/RNS_sharing/GB_5x5_shared)
add_circuit(NN_4_shared ../Benchmarks/RNS_sharing/NN_4_shared)
add_circuit(NN_8_shared ../Benchmarks/RNS_sharing/NN_8_shared)
