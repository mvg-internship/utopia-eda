//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/checker.h"
#include "gate/model/gnet_test.h"

#include "gate/premapper/migmapper.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using Checker = eda::gate::debugger::Checker;
using GateBinding = Checker::GateBinding;
using GateIdMap = eda::gate::premapper::MigMapper::GateIdMap;
using Hints = Checker::Hints;
using Link = eda::gate::model::Gate::Link;
using MigMapper = eda::gate::premapper::MigMapper;

bool equivalenceCheck(const std::shared_ptr<GNet> &net,
                      const std::shared_ptr<GNet> &migMapped) {
  Checker checker;
  GateIdMap oldToNewGates;
  GateBinding inputBind;
  GateBinding outputBind;
  GateBinding triggerBind;

  assert(net->nSourceLinks() == migMapped->nSourceLinks()
         && "The number of source links error\n");
  assert(net->nTargetLinks() == migMapped->nTargetLinks()
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
  
  return checker.areEqual(*net, *migMapped, hints);
}

void migMap(const std::shared_ptr<GNet> &net) {
  dump(*net);
  MigMapper migmapper;
  auto migMapped = migmapper.map(*net);
  dump(*migMapped);
  migMapped->sortTopologically();

  // equivalence
  bool isEqual = equivalenceCheck(net, migMapped);
  std::cout << "equivalence: " << isEqual << '\n';
  EXPECT_TRUE(isEqual);
}

TEST(MigMapperTest, MigMapperOrTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOr(3, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperAndTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAnd(2, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf3Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(3, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf5Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(5, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf7Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(7, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf9Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(9, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf11Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(11, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMajOf17Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(17, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperNorTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNor(2, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperNandTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNand(2, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperOrnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOrn(2, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperAndnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAndn(2, inputs, outputId);
  migMap(net);
  EXPECT_TRUE(net != nullptr);
}
