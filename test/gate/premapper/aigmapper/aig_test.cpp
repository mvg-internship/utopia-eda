//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/checker.h"
#include "gtest/gtest.h"
#include "gate/premapper/mapper/mapper_test.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <random>

// The tests check the equivalence of nets with single gates and their premapped nets

TEST(AigPremapperTest, AigNotOneInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::NOT, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndThreeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::AND, 3);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrThreeInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 3);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrFourInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::OR, 4);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigXorTwoInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigXorFourInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 4);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigNotOneInvertedInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::NOT, 1);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndTwoInvertedInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigAndThreeInvertedInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::AND, 3);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrTwoInvertedInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigOrFourInvertedInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::OR, 4);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigXorTwoInvertedInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 2);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}

TEST(AigPremapperTest, AigXorFourInvertedInputTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 4);
  GateIdMap gmap;
  std::shared_ptr<GNet> premapped = premap(net, gmap, PreBasis::AIG);
  EXPECT_TRUE(checkEquivalence(net, premapped, gmap));
}
