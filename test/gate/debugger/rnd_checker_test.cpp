//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//
#include <iostream>
#include "gate/model/gnet.h"
#include "gate/debugger/rnd_checker.h"
#include "gate/model/gnet_test.h"
#include "gtest/gtest.h"
#include "gate/debugger/rnd_checker.cpp"

using namespace eda::gate::model;
using namespace eda::gate::debugger;

TEST(rnd_generatorTest, test1) {

  Gate::SignalList inputs;
  Gate::Id output;

  auto net = makeNor(8, inputs, output);

    std::cout<<"STARTING RND_GENERATOR TEST\n";
    int a = Generator(*net, 0, true);
    std::cout<<"GENERATOR RESULT IS: \t" << a << std::endl;
    EXPECT_TRUE(a == 0);
}




