//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/miter.h"
#include "gate/model/gnet_test.h"

#include "gtest/gtest.h"

using namespace eda::gate::debugger;
using namespace eda::gate::model;

TEST(MiterTest, MiterOutTest) {
  auto net = makeRand(7, 5);
  std::unordered_map<Gate::Id, Gate::Id> testMap = {};
  auto netCloned = net.get()->clone(testMap);
  GateBinding ibind, obind, tbind;

  // Input-to-input correspondence.
  for (auto oldSourceLink : net.get()->sourceLinks()) {
    auto newSourceId = testMap[oldSourceLink.target];
    ibind.insert({oldSourceLink, Gate::Link(newSourceId)});
  }

  // Output-to-output correspondence.
  for (auto oldTargetLink : net.get()->targetLinks()) {
    auto newTargetId = testMap[oldTargetLink.source];
    obind.insert({oldTargetLink, Gate::Link(newTargetId)});
  }

  // Trigger-to-trigger correspondence.
  for (auto oldTriggerId : net.get()->triggers()) {
    auto newTriggerId = testMap[oldTriggerId];
    tbind.insert({Gate::Link(oldTriggerId), Gate::Link(newTriggerId)});
  }

  Checker::Hints hints;
  hints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tbind));
  
  GNet * mit = miter(net.get(), netCloned, hints);
  EXPECT_TRUE(mit->nTargetLinks() == 1);
  EXPECT_TRUE(mit->nSourceLinks() == netCloned->nSourceLinks());
}


