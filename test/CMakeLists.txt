# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.0)

project(UtopiaTest VERSION 0.0)

set(TEST_TARGET utest)

# Check if the 'gtest' target has been already imported (e.g., by LLVM).
if(NOT TARGET gtest)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
  )

  # For Windows: Prevent overriding the parent project's compiler/linker settings.
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

enable_testing()

add_executable(utest
  gate/debugger/checker_test.cpp
  gate/model/gnet_test.cpp
  gate/simulator/simulator_test.cpp
  lib/minisat/minisat_test.cpp
  rtl/parser/ril/ril_test.cpp
  util/fm_test.cpp
  test_main.cpp
)

target_include_directories(utest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(utest
  PRIVATE
    Utopia::Lib
    gtest_main
    easyloggingpp
)

include(GoogleTest)
gtest_discover_tests(utest)

file(COPY data DESTINATION .)
