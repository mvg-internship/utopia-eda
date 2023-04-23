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
add_executable(parserRIL src/gate/parserRIL/main.cpp)
target_link_libraries(parserRIL PRIVATE Utopia::Lib Yosys::Yosys)
