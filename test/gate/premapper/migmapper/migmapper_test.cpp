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
using Gate = eda::gate::model::Gate;
using GateBinding = Checker::GateBinding;
using GateIdMap = eda::gate::premapper::MigMapper::GateIdMap;
using GNet = eda::gate::model::GNet;
using Hints = Checker::Hints;
using Link = eda::gate::model::Gate::Link;
using MigMapper = eda::gate::premapper::MigMapper;

void initializeMigBinds(const GNet &net,
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

std::shared_ptr<GNet> migMap(std::shared_ptr<GNet> net, GateIdMap &gmap) {
  eda::gate::premapper::MigMapper migMapper;
  std::shared_ptr<GNet> migMapped = migMapper.map(*net, gmap);
  //dump(*net);
  //dump(*migMapped);
  migMapped->sortTopologically();
  return migMapped;
}

bool checkMigEquivalence(const std::shared_ptr<GNet> net,
                         const std::shared_ptr<GNet> migMapped,
                         GateIdMap &gmap) {
  // Initialize binds
  GateBinding ibind, obind;
  initializeMigBinds(*net, gmap, ibind, obind);
  eda::gate::debugger::Checker::Hints hints;
  hints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  // check equivalence
  eda::gate::debugger::Checker checker;
  bool equal = checker.areEqual(*net, *migMapped, hints);
  return equal;
}

TEST(MigMapperTest, MigMapperOrTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOr(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperAndTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAnd(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperMajOf3Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(3, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperMajOf5Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(5, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperMajOf7Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(7, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
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
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperNandTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNand(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperOrnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOrn(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}

TEST(MigMapperTest, MigMapperAndnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAndn(1024, inputs, outputId);
  GateIdMap gmap;
  std::shared_ptr<GNet> migMapped = migMap(net, gmap);
  EXPECT_TRUE(checkMigEquivalence(net, migMapped, gmap));
}