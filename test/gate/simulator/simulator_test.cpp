//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet_test.h"
#include "gate/simulator/simulator.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace eda::gate::model;
using namespace eda::gate::simulator;

static Simulator simulator;

static bool simulatorTest(std::uint64_t N,
                          const GNet &net,
                          Gate::SignalList inputs,
                          Gate::Id output) {
  GNet::LinkList in;
  GNet::LinkList out{Gate::Link(output)};

  for (auto input : inputs) {
    in.push_back(Gate::Link(input.node()));
  }

  auto compiled = simulator.compile(net, in, out);

  std::uint64_t o;
  for (std::uint64_t i = 0; i < N; i++) {
    compiled.simulate(o, i);
    std::cout << std::hex << i << " -> " << o << std::endl;
  }

  return true;
}

bool simulatorNorTest(unsigned N) {
  // ~(x1 | ... | xN).
  Gate::SignalList inputs;
  Gate::Id output;

  auto net = makeNor(N, inputs, output);

  return simulatorTest(1ull << N, *net, inputs, output);
}

bool simulatorAndnTest(unsigned N) {
  // (~x1 & ... & ~xN).
  Gate::SignalList inputs;
  Gate::Id output;

  auto net = makeAndn(N, inputs, output);

  return simulatorTest(1ull << N, *net, inputs, output);
}

TEST(SimulatorGNetTest, SimulatorNorTest) {
  EXPECT_TRUE(simulatorNorTest(4));
}

TEST(SimulatorGNetTest, SimulatorAndnTest) {
  EXPECT_TRUE(simulatorAndnTest(4));
}
