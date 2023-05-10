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

/*
 * Check basis gate correct mapping
 */

TEST(AigPremapperTest, AigNotOneInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::NOT, 1);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndTwoInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndThreeInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 3);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

/*
 * Check out of basis gate correct mapping
 */

TEST(AigPremapperTest, AigOrTwoInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrThreeInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 3);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrFourInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigXorTwoInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigXorFourInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

/*
 * Check basis gate inverted inputs correct mapping
 */

TEST(AigPremapperTest, AigNotOneInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::NOT, 1);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndTwoInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndThreeInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 3);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

/*
 * Check out of basis gate inverted inputs correct mapping
 */

TEST(AigPremapperTest, AigOrTwoInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrFourInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}


TEST(AigPremapperTest, AigXorTwoInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 2);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigXorFourInvertedInputTest) {
  // Create network
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 4);
  // Premapping
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  // Check equivalence
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}
