# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

project(Utopia VERSION 0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
find_package(Yosys)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)
add_executable(LibertyParser src/gate/Liberty/main.cpp)
target_link_libraries(LibertyParser PRIVATE Utopia::Lib Yosys::Yosys)
add_executable(VerilogParser src/gate/Verilog/main.cpp)
target_link_libraries(VerilogParser PRIVATE Utopia::Lib Yosys::Yosys)
add_executable(BlifParser src/gate/Blif/main.cpp)
target_link_libraries(BlifParser PRIVATE Utopia::Lib Yosys::Yosys)
add_executable(table src/gate/tool/main.cpp)
target_link_libraries(table PRIVATE Utopia::Lib Yosys::Yosys)
add_executable(parserRIL src/gate/parserRIL/main.cpp)
target_link_libraries(parserRIL PRIVATE Utopia::Lib Yosys::Yosys)
add_executable(parserGnet src/gate/parserGnet/main.cpp)
target_link_libraries(parserGnet PRIVATE Utopia::Lib Yosys::Yosys)
