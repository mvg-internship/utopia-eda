//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//
#include <iostream>

#include "gate/debugger/rnd_checker.h"
#include "gate/model/gnet_test.h"
#include "gtest/gtest.h"

using namespace eda::gate::debugger;
using namespace eda::gate::model;

TEST(rnd_checkerTest, SimpleTest) {

  Gate::SignalList inputs;
  Gate::Id output;

  auto net = makeNor(8, inputs, output);

  std::cout << "STARTING RND_CHECKER TEST\n";
  int a = rndChecker(*net, 0, true);
  std::cout << "CHECKER RESULT IS: \t" << a << std::endl;
  EXPECT_TRUE(a == 0);
}
