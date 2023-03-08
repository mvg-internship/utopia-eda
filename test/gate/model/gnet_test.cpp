//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet_test.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <random>

using namespace eda::gate::model;

// gate(x1, ..., xN).
static std::unique_ptr<GNet> makeNet(GateSymbol gate,
                                     unsigned N,
                                     Gate::SignalList &inputs,
                                     Gate::Id &outputId) {
  auto net = std::make_unique<GNet>();

  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->addIn();
    inputs.push_back(Gate::Signal::always(inputId));
  }

  auto gateId = net->addGate(gate, inputs);
  outputId = net->addOut(gateId);

  net->sortTopologically();
  return net;
}

// gate(~x1, ..., ~xN).
static std::unique_ptr<GNet> makeNetn(GateSymbol gate,
                                      unsigned N,
                                      Gate::SignalList &inputs,
                                      Gate::Id &outputId) {
  auto net = std::make_unique<GNet>();

  Gate::SignalList andInputs;
  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->addIn();
    inputs.push_back(Gate::Signal::always(inputId));

    const Gate::Id notGateId = net->addNot(inputId);
    andInputs.push_back(Gate::Signal::always(notGateId));
  }

  auto gateId = net->addGate(gate, inputs);
  outputId = net->addOut(gateId);

  net->sortTopologically();
  return net;
}

// (x1 | ... | xN).
std::unique_ptr<GNet> makeOr(unsigned N,
                             Gate::SignalList &inputs,
                             Gate::Id &outputId) {
  return makeNet(GateSymbol::OR, N, inputs, outputId);
}

// (x1 & ... & xN).
std::unique_ptr<GNet> makeAnd(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId) {
  return makeNet(GateSymbol::AND, N, inputs, outputId);
}


// ~(x1 | ... | xN).
std::unique_ptr<GNet> makeNor(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId) {
  return makeNet(GateSymbol::NOR, N, inputs, outputId);
}

// ~(x1 & ... & xN).
std::unique_ptr<GNet> makeNand(unsigned N,
                               Gate::SignalList &inputs,
                               Gate::Id &outputId) {
  return makeNet(GateSymbol::NAND, N, inputs, outputId);
}


// (~x1 | ... | ~xN).
std::unique_ptr<GNet> makeOrn(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId) {
  return makeNetn(GateSymbol::OR, N, inputs, outputId);
}

// (~x1 & ... & ~xN).
std::unique_ptr<GNet> makeAndn(unsigned N,
                               Gate::SignalList &inputs,
                               Gate::Id &outputId) {
  return makeNetn(GateSymbol::AND, N, inputs, outputId);
}

// Random hierarchical network.
std::unique_ptr<GNet> makeRand(std::size_t nGates,
                               std::size_t nSubnets) {
  assert(nGates >= 2);
  auto net = std::make_unique<GNet>();

  // Create subnets.
  for (std::size_t i = 0; i < nSubnets; i++) {
    net->newSubnet();
  }

  // Create empty gates.
  const auto minGateId = net->newGate();
  for (std::size_t i = 0; i < nGates - 2; i++) {
    net->newGate();
  }
  const auto maxGateId = net->newGate();

  std::mt19937 gen(0);
  std::uniform_int_distribution<Gate::Id> snetDist(0, nSubnets - 1);
  std::uniform_int_distribution<Gate::Id> gateDist(minGateId, maxGateId);

  std::size_t minArity = 0u;
  std::size_t maxArity = std::min(static_cast<std::size_t>(7), nGates - 1);
  std::uniform_int_distribution<std::size_t> arityDist(minArity, maxArity);

  for (std::size_t n = 0; n < 4; n++) {
    // Create subnets.
    for (std::size_t i = 0; i < nSubnets; i++) {
      net->newSubnet();
    } // for: create subnets.

    // Randomly distributes the gates among the subnets.
    for (std::size_t i = 0; i < nGates; i++) {
      const auto gid = gateDist(gen);
      const auto dst = snetDist(gen);

      if (net->contains(gid)) {
        net->moveGate(gid, dst);
      }
    } // for: move gates.

    // Randomly modify/connect the gates.
    for (std::size_t i = 0; i < nGates; i++) {
      const auto gid = gateDist(gen);
      if (!net->contains(gid)) {
        continue;
      }

      Gate::SignalList inputs;

      const std::size_t arity = arityDist(gen);
      for (std::size_t j = 0; j < arity; j++) {
        const auto inputId = gateDist(gen);

        if (net->contains(inputId)) {
          const auto input = Gate::Signal::always(inputId);
          inputs.push_back(input);
        }
      } // for: arity

      // Beware of combinational cycles.
      if (!net->hasCombFlow(gid, inputs)) {
        const auto func = (inputs.empty() ? GateSymbol::IN : GateSymbol::AND);
        net->setGate(gid, func, inputs);
      }
    } // for: set gates.

    // Randomly remove some gates.
    for (std::size_t i = 0; i < nGates / 16; i++) {
      const auto gid = gateDist(gen);
      if (!net->contains(gid)) {
        continue;
      }

      auto *source = Gate::get(gid);

      // Check the dependent gates.
      for (const auto link : source->links()) {
        auto *target = Gate::get(link.target);
        if (!net->contains(target->id())) {
          continue;
        }

        bool allFromSource = true;
        for (const auto input : target->inputs()) {
          if (input.node() != source->id()) {
            allFromSource = false;
            break;
          }
        }

        // If a gate depends solely on the gate being removed,
        // it will have no inputs (became an input).
        if (allFromSource) {
          net->setIn(target->id());
        }
      } // for: source links.

      net->removeGate(gid);
    } // for: remove gate.

    net->groupOrphans();
    net->removeEmptySubnets();

    net->sortTopologically();

    net->flatten();
  } // for: top-level.

  return net;
}

TEST(GNetTest, GNetOrTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOr(1024, inputs, outputId);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetAndTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAnd(1024, inputs, outputId);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetNorTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNor(1024, inputs, outputId);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetNandTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNand(1024, inputs, outputId);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetOrnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOrn(1024, inputs, outputId);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetAndnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAndn(1024, inputs, outputId);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetRandTest) {
  auto net = makeRand(1024, 256);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetRandTestIssue11877) {
  auto net = makeRand(7, 5);
  EXPECT_TRUE(net != nullptr);
}

TEST(GNetTest, GNetWithCheckerTest) {
  eda::gate::debugger::Checker checker;
  auto net = makeRand(7, 5);
  std::unordered_map<Gate::Id, Gate::Id> testMap = {};
  auto netCloned = net.get()->clone(testMap);
  EXPECT_TRUE(checker.areEqual(*net, *netCloned, testMap));
}

TEST(GNetTest, GNetEdgesTest) {
  auto net = makeRand(7, 5);
  EXPECT_TRUE(net.get()->clone()->nEdges() == net.get()->nEdges());
}

TEST(GNetTest, GNetAddressTest) {
  auto net = makeRand(7, 5);
  EXPECT_TRUE(net.get()->clone() != net.get());
}

