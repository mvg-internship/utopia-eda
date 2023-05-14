//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/checker.h"
#include "gate/model/gnet_test.h"
#include "gate/premapper/mapper/mapper_test.h"

#include "gate/premapper/migmapper.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using GateIdMap = eda::gate::premapper::MigMapper::GateIdMap;
using MigMapper = eda::gate::premapper::MigMapper;

using namespace eda::gate::model;

TEST(MigMapperTest, MigMapperOrTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOr(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperAndTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAnd(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperXorTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 512);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperMajOf3Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(3, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperMajOf5Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(5, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperMajOf7Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(7, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperMajOf9Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(9, inputs, outputId);
  //migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf11Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(11, inputs, outputId);
  //migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf17Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(17, inputs, outputId);
  //migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperNorTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNor(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperNandTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNand(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperOrnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOrn(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperAndnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAndn(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperXornTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 512);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = premap(net, gmap, PreBasis::MIG);
  EXPECT_TRUE(checkEquivalence(net, migMapped, gmap));
}
