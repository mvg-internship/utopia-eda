//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/checker.h"
#include "gate/model/gnet_test.h"

#include "gate/premapper/xmgmapper.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using Checker = eda::gate::debugger::Checker;
using Gate = eda::gate::model::Gate;
using GateBinding = Checker::GateBinding;
using GateIdMap = eda::gate::premapper::MigMapper::GateIdMap;
using GNet = eda::gate::model::GNet;
using Hints = Checker::Hints;
using Link = eda::gate::model::Gate::Link;
using XmgMapper = eda::gate::premapper::XmgMapper;

bool xmgEquivalenceCheck(const std::shared_ptr<GNet> &net,
                         const std::shared_ptr<GNet> &xmgMapped) {
  Checker checker;
  GateIdMap oldToNewGates;
  GateBinding inputBind;
  GateBinding outputBind;
  GateBinding triggerBind;

  assert(net->nSourceLinks() == xmgMapped->nSourceLinks()
         && "The number of source links error\n");
  assert(net->nTargetLinks() == xmgMapped->nTargetLinks()
         && "The number of target links error\n");

  // Input-to-input correspondence
  for (const auto oldSourceLink : net->sourceLinks()) {
    auto newSourceId = oldToNewGates[oldSourceLink.target];
    inputBind.insert({oldSourceLink, Link(newSourceId)});
  }

  // Output-to-output correspondence
  for (const auto oldTargetLink : net->targetLinks()) {
    auto newTargetId = oldToNewGates[oldTargetLink.source];
    outputBind.insert({oldTargetLink, Link(newTargetId)});
  }

  // Trigger-to-trigger correspondence
  for (const auto oldTriggerId : net->triggers()) {
    auto newTriggerId = oldToNewGates[oldTriggerId];
    triggerBind.insert({Link(oldTriggerId), Link(newTriggerId)});
  }

  Hints hints;
  hints.sourceBinding = std::make_shared<GateBinding>(std::move(inputBind));
  hints.targetBinding = std::make_shared<GateBinding>(std::move(outputBind));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(triggerBind));

  return checker.areEqual(*net, *xmgMapped, hints);
}

void xmgMap(const std::shared_ptr<GNet> &net) {
  dump(*net);
  XmgMapper xmgmapper;
  auto xmgMapped = xmgmapper.map(*net);
  dump(*xmgMapped);
  xmgMapped->sortTopologically();

  // equivalence
  bool isEqual = xmgEquivalenceCheck(net, xmgMapped);
  std::cout << "equivalence: " << isEqual << '\n';
  EXPECT_TRUE(isEqual);
}

TEST(XmgMapperTest, XmgMapperOrTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOr(3, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(XmgMapperTest, XmgMapperAndTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAnd(2, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(XmgMapperTest, XmgMapperMajOf3Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(3, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(XmgMapperTest, XmgMapperMajOf5Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(5, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(XmgMapperTest, XmgMapperNorTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNor(2, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(XmgMapperTest, XmgMapperNandTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNand(2, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(XmgMapperTest, XmgMapperOrnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOrn(2, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(XmgMapperTest, XmgMapperAndnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAndn(2, inputs, outputId);
  xmgMap(net);
  EXPECT_TRUE(net != nullptr);
}
