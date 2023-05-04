//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/aigmapper/aig_test.h"
#include "gate/debugger/checker.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <random>

using namespace eda::gate::premapper;

std::shared_ptr<GNet> premapAig(std::shared_ptr<GNet> net, GateIdMap &gmap) {
  eda::gate::premapper::AigMapper premapper;
  std::shared_ptr<GNet> premapped = premapper.map(*net, gmap);
  premapped->sortTopologically();
  return premapped;
}

/*
 * Check basis gate correct mapping
 */

TEST(AigPremapperTest, AigNotOneInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::NOT, 1);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigAndTwoInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigAndThreeInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 3);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

/*
 * Check out of basis gate correct mapping
 */

TEST(AigPremapperTest, AigOrTwoInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigOrFourInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigXorTwoInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigXorFourInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

/*
 * Check basis gate inverted inputs correct mapping
 */

TEST(AigPremapperTest, AigNotOneInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::NOT, 1);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigAndTwoInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigAndThreeInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 3);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

/*
 * Check out of basis gate inverted inputs correct mapping
 */

TEST(AigPremapperTest, AigOrTwoInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigOrFourInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigXorTwoInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}

TEST(AigPremapperTest, AigXorFourInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premapAig(net, gmap);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap) == 1);
}
