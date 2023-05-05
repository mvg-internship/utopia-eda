//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/miter.h"

namespace eda::gate::debugger {

bool areMiterable(GNet &net1, GNet &net2, Hints &hints) {
  if (net1.nSourceLinks() != net2.nSourceLinks()) {
    CHECK(false) << "Nets do not have the same number of inputs\n";
    return false;
  }

  for (auto sourceLink : net1.sourceLinks()) {
    if (hints.sourceBinding.get()->find(sourceLink) == hints.sourceBinding.get()->end()) {
      CHECK(false) << "Unable to find source with id " << sourceLink.target << '\n';
      return false;
    }
  }
  for (auto targetLink : net1.targetLinks()) {
    if (hints.targetBinding.get()->find(targetLink) == hints.sourceBinding.get()->end()) {
      CHECK(false) << "Unable to find target with id " << targetLink.source << '\n';
      return false;
    }
  }

  return true;
}

GNet *miter(GNet &net1, GNet &net2, Hints &hints) {
  if (not areMiterable(net1, net2, hints)) {
    return nullptr;
  }

  std::unordered_map<Gate::Id, Gate::Id> map1 = {};
  std::unordered_map<Gate::Id, Gate::Id> map2 = {};
  GNet *cloned1 = net1.clone(map1);
  GNet *cloned2 = net2.clone(map2);

  GateBinding ibind, obind, tbind;
  // Input-to-input correspondence.
  for (auto bind : *hints.sourceBinding.get()) {
    auto newSourceId1 = map1[bind.first.target];
    auto newSourceId2 = map2[bind.second.target];
    ibind.insert({Gate::Link(newSourceId1), Gate::Link(newSourceId2)});
  }

  // Output-to-output correspondence.
  for (auto bind : *hints.targetBinding.get()) {
    auto newTargetId1 = map1[bind.first.source];
    auto newTargetId2 = map2[bind.second.source];
    obind.insert({Gate::Link(newTargetId1), Gate::Link(newTargetId2)});
  }

  // Trigger-to-trigger correspondence.
  for (auto bind : *hints.triggerBinding.get()) {
    auto newTriggerId1 = map1[bind.first.source];
    auto newTriggerId2 = map2[bind.second.source];
    tbind.insert({Gate::Link(newTriggerId1), Gate::Link(newTriggerId2)});
  }

  Checker::Hints newHints;
  newHints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  newHints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  newHints.triggerBinding = std::make_shared<GateBinding>(std::move(tbind));

  SignalList inputs, xorSignalList;
  GNet *miter = new GNet();
  miter->addNet(*cloned1);
  miter->addNet(*cloned2);

  for (auto bind : *newHints.sourceBinding.get()) {
    GateId newInputId = miter->addIn();
    if (Gate::get(bind.first.target)->func() == GateSymbol::IN) {
      miter->setGate(bind.first.target, GateSymbol::NOP, newInputId);
      miter->setGate(bind.second.target, GateSymbol::NOP, newInputId);
    } else {
      miter->setGate(bind.first.target, Gate::get(bind.first.target)->func(), newInputId);
      miter->setGate(bind.second.target, Gate::get(bind.second.target)->func(), newInputId);
    }
  }

  for (auto bind : *newHints.targetBinding.get()) {
    GateId newOutId = miter->addGate(GateSymbol::XOR, bind.first.source, bind.second.source);
    xorSignalList.push_back(Signal::always(newOutId));
  }

  GateId finalOutId = miter->addOr(xorSignalList);
  miter->addOut(finalOutId);
  miter->setGate(finalOutId, GateSymbol::OR, xorSignalList);

  for (auto bind : *newHints.targetBinding.get()) {
    miter->setGate(bind.first.source, GateSymbol::NOP, Gate::get(bind.first.source)->inputs());
    miter->setGate(bind.second.source, GateSymbol::NOP, Gate::get(bind.second.source)->inputs());
  }

  miter->sortTopologically();
  return miter;
}
} // namespace eda::gate::debugger
