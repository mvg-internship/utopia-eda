//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/xagmapper/xag_test.h"
#include "gate/debugger/checker.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <random>

using namespace eda::gate::premapper;

using Gate = eda::gate::model::Gate;
using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
using GateIdMap = std::unordered_map<Gate::Id, Gate::Id>;
using GNet = eda::gate::model::GNet;
using Link = Gate::Link;

// gate(x1, ..., xN).
std::shared_ptr<GNet> makeSingleGateNet(GateSymbol gate,
                                        const unsigned N) {
  std::shared_ptr<GNet> net = std::make_shared<GNet>();

  Gate::SignalList inputs;

  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->addIn();
    inputs.push_back(Gate::Signal::always(inputId));
  }

  auto gateId = net->addGate(gate, inputs);
  net->addOut(gateId);

  net->sortTopologically();
  return net;
}

// gate(~x1, ..., ~xN).
std::shared_ptr<GNet> makeSingleGateNetn(GateSymbol gate,
                                         const unsigned N) {
  std::shared_ptr<GNet> net = std::make_shared<GNet>();

  Gate::SignalList inputs;

  Gate::SignalList andInputs;
  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->addIn();
    inputs.push_back(Gate::Signal::always(inputId));

    const Gate::Id notGateId = net->addNot(inputId);
    andInputs.push_back(Gate::Signal::always(notGateId));
  }

  auto gateId = net->addGate(gate, andInputs);
  net->addOut(gateId);

  net->sortTopologically();
  return net;
}

void initializeBinds(const GNet &net,
                     GateIdMap &gmap,
                     GateBinding &ibind,
                     GateBinding &obind) {
  // Input-to-input correspondence.
  for (const auto oldSourceLink : net.sourceLinks()) {
    auto newSourceId = gmap[oldSourceLink.target];
    ibind.insert({oldSourceLink, Link(newSourceId)});
  }

  // Output-to-output correspondence.
  for (const auto oldTargetLink : net.targetLinks()) {
    auto newTargetId = gmap[oldTargetLink.source];
    obind.insert({oldTargetLink, Link(newTargetId)});
  }
}

std::shared_ptr<GNet> premap(std::shared_ptr<GNet> net, GateIdMap &gmap) {
  eda::gate::premapper::XagMapper premapper;
  std::shared_ptr<GNet> premapped = premapper.map(*net, gmap);
  premapped->sortTopologically();
  return premapped;
}

bool checkEquivalence(const std::shared_ptr<GNet> net,
                      const std::shared_ptr<GNet> premapped,
                      GateIdMap &gmap) {
  // Initialize binds
  GateBinding ibind, obind;
  initializeBinds(*net, gmap, ibind, obind);
  eda::gate::debugger::Checker::Hints hints;
  hints.sourceBinding = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding = std::make_shared<GateBinding>(std::move(obind));
  //check equivalence
  eda::gate::debugger::Checker checker;
  bool equal = checker.areEqual(*net, *premapped, hints);
  return equal;
}

/*
 * Check basis gate correct mapping
 */

// Not gate tests
TEST(XagPremapperTest, XagNotOneInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::NOT, 1);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

// And gate tests
TEST(XagPremapperTest, XagAndOneInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(XagPremapperTest, XagAndTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

// Xor gate tests
TEST(XagPremapperTest, XagXorOneInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(XagPremapperTest, XagXorTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

/*
 * Check out of basis gates correct mapping
 */

// Not gate tests
TEST(XagPremapperTest, XagOrOneInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(XagPremapperTest, XagOrTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

/*
 * Negative (inverted) inputs basis gates
 */

// Not gate test with inverted inputs
TEST(XagPremapperTest, XagNotOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::NOT, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

// And gate tests with inverted inputs
TEST(XagPremapperTest, XagAndOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(XagPremapperTest, XagAndTwoNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

// Xor gate tests with inverted inputs
TEST(XagPremapperTest, XagXorOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(XagPremapperTest, XagXorTwoNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

/*
 * Check out of basis gates (inverted inputs) correct mapping
 */

// Or gate tests with inverted inputs
TEST(XagPremapperTest, XagOrOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(XagPremapperTest, XagOrTwoNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}


