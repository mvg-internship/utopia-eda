//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/mapper/mapper_test.h"
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

/*
 * Check basis gate correct mapping
 */

// Not gate tests
TEST(XagPremapperTest, XagNotOneInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::NOT, 1);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

// And gate tests
TEST(XagPremapperTest, XagAndOneInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(XagPremapperTest, XagAndTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

// Xor gate tests
TEST(XagPremapperTest, XagXorOneInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(XagPremapperTest, XagXorTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

/*
 * Check out of basis gates correct mapping
 */

// Not gate tests
TEST(XagPremapperTest, XagOrOneInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(XagPremapperTest, XagOrTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

/*
 * Negative (inverted) inputs basis gates
 */

// Not gate test with inverted inputs
TEST(XagPremapperTest, XagNotOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::NOT, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

// And gate tests with inverted inputs
TEST(XagPremapperTest, XagAndOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(XagPremapperTest, XagAndTwoNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

// Xor gate tests with inverted inputs
TEST(XagPremapperTest, XagXorOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(XagPremapperTest, XagXorTwoNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

/*
 * Check out of basis gates (inverted inputs) correct mapping
 */

// Or gate tests with inverted inputs
TEST(XagPremapperTest, XagOrOneNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(XagPremapperTest, XagOrTwoNegativeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}


