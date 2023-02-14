//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "rtl/library/arithmetic.h"
#include "gate/simulator/simulator.h"

#include "gtest/gtest.h"

#include <iostream>

using GNet = eda::gate::model::GNet;
using FuncSymbol = eda::rtl::model::FuncSymbol;
using FLibrary = eda::rtl::library::FLibrary;
using ArithmeticLibrary = eda::rtl::library::ArithmeticLibrary;
using Simulator = eda::gate::simulator::Simulator;

bool ADDTest(const size_t xSize,
             const size_t ySize,
             const size_t outSize) {
  GNet::GateIdList x(xSize);
  GNet::GateIdList y(ySize);
  GNet net;
  for (size_t n = 0; n < xSize; n++) {
    x[n] = net.addIn();
  }
  for (size_t n = 0; n < ySize; n++) {
    y[n] = net.addIn();
  }

  GNet::In inputs = {x, y};
  FLibrary &library = ArithmeticLibrary::get();
  GNet::GateIdList outputs = library.synth(outSize, FuncSymbol::ADD, inputs, net);
  net.sortTopologically();

  GNet::LinkList in;
  for (size_t n = 0; n < xSize; n++) {
    in.push_back(GNet::Link(x[n]));
  }
  for (size_t n = 0; n < ySize; n++) {
    in.push_back(GNet::Link(y[n]));
  }

  GNet::LinkList out;
  for (size_t n = 0; n < outSize; n++) {
    out.push_back(Gate::Link(outputs[n]));
  }

  Simulator simulator;
  auto compiled = simulator.compile(net, in, out);

  std::vector<bool> o(outSize);
  std::vector<bool> i(xSize + ySize);
  for (size_t n = 0; n < xSize + ySize; n++)
     i[n] = rand() % 2;
  compiled.simulate(o, i);

  
  for (int n = xSize - 1; n >= 0; n--) {
    std::cout << i[n];
  }
  std::cout << " + ";
  for (int n = xSize + ySize - 1; n >= x.size(); n--) {
    std::cout << i[n];
  }
  std::cout << " = ";

  for (int n = outputs.size() - 1; n >= 0; n--) {
    std::cout << o[n];
  }
  std::cout << std::endl;
  return true;
}

TEST(ArithmeticTest, ADDTest_1) {
  EXPECT_TRUE(ADDTest(7, 7, 50));
}
TEST(ArithmeticTest, ADDTest_2) {
  EXPECT_TRUE(ADDTest(10, 15, 7));
}
TEST(ArithmeticTest, ADDTest_3) {
    EXPECT_TRUE(ADDTest(15, 15, 16));
}
TEST(ArithmeticTest, ADDTest_4) {
  EXPECT_TRUE(ADDTest(15, 15, 15));
}
TEST(ArithmeticTest, ADDTest_5) {
    EXPECT_TRUE(ADDTest(17, 4, 10));
}
TEST(ArithmeticTest, ADDTest_6) {
  EXPECT_TRUE(ADDTest(8, 8, 7));
}
TEST(ArithmeticTest, ADDTest_7) {
    EXPECT_TRUE(ADDTest(7, 14, 8));
}
TEST(ArithmeticTest, ADDTest_8) {
  EXPECT_TRUE(ADDTest(14, 7, 7));
}
TEST(ArithmeticTest, ADDTest_9) {
    EXPECT_TRUE(ADDTest(23, 28, 20));
}
TEST(ArithmeticTest, ADDTest_10) {
  EXPECT_TRUE(ADDTest(1, 1, 1));
}
