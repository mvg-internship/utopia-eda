//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/bdd_checker.h"
#include "gate/model/gnet_test.h"

#include "gtest/gtest.h"

using namespace eda::gate::debugger;
using namespace eda::gate::model;

TEST(BDDCheckerTest, BDDCheckerTest) {
  GNet *net = new GNet();
  SignalList inps1;
  int nAndInputs = 100;
  for (int i = 0; i < nAndInputs; i++) {
    GateId inId1 = net->addIn();
    inps1.push_back(Signal::always(inId1));
  }
  GateId andId = net->addGate(GateSymbol::AND, inps1);
  GateId outId = net->addOut(andId);
  SignalList inps2;
  int nNorInputs = 100;
  for (int i = 0; i < nNorInputs; i++) {
    GateId inId2 = net->addIn();
    inps2.push_back(Signal::always(inId2));
  }
  GateId norId = net->addGate(GateSymbol::NOR, inps2);
  net->addOut(norId);
  int nOrOutputs = 100;
  GateId orId = net->addGate(GateSymbol::OR, inps1);
  for (int i = 0; i < nOrOutputs; i++) {
    outId = net->addOut(orId);
  }
  net->addOut(orId);
  std::unordered_map<Gate::Id, Gate::Id> testMap = {};
  GNet* netCloned = net->clone(testMap);
  
  GateBinding ibind, obind, tbind;

  // Input-to-input correspondence.
  for (auto oldSourceLink : net->sourceLinks()) {
    auto newSourceId = testMap[oldSourceLink.target];
    ibind.insert({oldSourceLink, Gate::Link(newSourceId)});
  }

  // Output-to-output correspondence.
  for (auto oldTargetLink : net->targetLinks()) {
    auto newTargetId = testMap[oldTargetLink.source];
    obind.insert({oldTargetLink, Gate::Link(newTargetId)});
  }

  // Trigger-to-trigger correspondence.
  for (auto oldTriggerId : net->triggers()) {
    auto newTriggerId = testMap[oldTriggerId];
    tbind.insert({Gate::Link(oldTriggerId), Gate::Link(newTriggerId)});
  }

  Checker::Hints hints;
  hints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tbind));
  
  EXPECT_TRUE(bddChecker(*net, *netCloned, hints));
}
