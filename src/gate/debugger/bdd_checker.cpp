//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/bdd_checker.h"

namespace eda::gate::debugger {

bool bddChecker(GNet &net1, GNet &net2, Hints &hints) {

  GNet *miterNet = miter(net1, net2, hints);
  SignalList inputs;
  GateId outputId = 0;

  Cudd manager(0, 0);
  for (Gate *gate : miterNet->gates()) {
    if (gate->func() == GateSymbol::IN) {
      inputs.push_back(Signal::always(gate->id()));
    }
    if (gate->func() == GateSymbol::OUT) {
      outputId = gate->id();
    }
  }
  BddList bddVarList = {};
  for (unsigned i = 0; i < inputs.size(); i++) {
    bddVarList.push_back(manager.bddVar());
  }
  
  GateBDDMap varMap;
  for (unsigned i = 0; i < bddVarList.size(); i++) {
    varMap[inputs[i].node()] = bddVarList[i];
  }

  BDD netBDD = GNetBDDConverter::convert(*miterNet, outputId, varMap, manager);
  return (netBDD == manager.bddZero());
} 

bool BddChecker::areEqual(GNet &lhs,
                          GNet &rhs,
                          Checker::GateIdMap &gmap) {

  GateBinding ibind, obind, tbind;

  // Input-to-input correspondence.
  for (auto oldSourceLink : lhs.sourceLinks()) {
    auto newSourceId = gmap[oldSourceLink.target];
    ibind.insert({oldSourceLink, Gate::Link(newSourceId)});
  }

  // Output-to-output correspondence.
  for (auto oldTargetLink : lhs.targetLinks()) {
    auto newTargetId = gmap[oldTargetLink.source];
    obind.insert({oldTargetLink, Gate::Link(newTargetId)});
  }

  // Trigger-to-trigger correspondence.
  for (auto oldTriggerId : lhs.triggers()) {
    auto newTriggerId = gmap[oldTriggerId];
    tbind.insert({Gate::Link(oldTriggerId), Gate::Link(newTriggerId)});
  }

  Checker::Hints hints;
  hints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tbind));

  return (bddChecker(lhs, rhs, hints));
}

} // namespace eda::gate::debugger
