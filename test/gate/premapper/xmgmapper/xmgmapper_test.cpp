//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/mapper/mapper_test.h"
#include "gate/debugger/checker.h"
#include "gate/model/gnet_test.h"

#include "gate/premapper/xmgmapper.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using GateIdMap = eda::gate::premapper::XmgMapper::GateIdMap;
using XmgMapper = eda::gate::premapper::XmgMapper;

TEST(XmgMapperTest, XmgMapperOrTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeOr(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperAndTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeAnd(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperXorTest) {
  std::shared_ptr<GNet> net = makeSingleGateNet(GateSymbol::XOR, 512);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperMajOf3Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeMaj(3, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperMajOf5Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeMaj(5, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperMajOf7Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeMaj(7, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperNorTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeNor(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperNandTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeNand(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperOrnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeOrn(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperAndnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = eda::gate::model::makeAndn(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}

TEST(XmgMapperTest, XmgMapperXornTest) {
  std::shared_ptr<GNet> net = makeSingleGateNetn(GateSymbol::XOR, 512);
  GateIdMap gmap;
  std::shared_ptr<GNet> xmgMapped = premap(net, gmap, PreBasis::XMG);
  EXPECT_TRUE(checkEquivalence(net, xmgMapped, gmap));
}