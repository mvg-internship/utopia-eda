//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/library/parser_lib.h"

#include "gtest/gtest.h"

using namespace eda::gate::library;

const std::string path = getenv("UTOPIA_HOME");

bool checkLibParser(std::string pathToLibJson) { 
  LibertyParser liberty;
  liberty.readJson(pathToLibJson);
  liberty.conversionToTruthTable();
  for(long unsigned int i = 0; i < liberty.cells.size(); i++) {
    kitty::print_binary(liberty.cells[i], std::cout);
    std::cout << '\n';
  }
  return true;
}

TEST(LibraryTest, checkLibParser) {
  EXPECT_TRUE(checkLibParser(path + "/test/gate/library/sky130_fd_sc_hd__ff_100C_1v65.json"));
}