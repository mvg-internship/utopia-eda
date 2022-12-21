//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/premapper.h"

#include <cassert>
#include <unordered_map>

namespace eda::gate::premapper {

Gate::SignalList getNewInputs(const Gate::SignalList &oldInputs,
                              const PreMapper::GateIdMap &oldToNewGates) {
  Gate::SignalList newInputs(oldInputs.size());

  for (size_t i = 0; i< oldInputs.size(); i++) {
    auto oldInput = oldInputs[i];
    auto newInput = oldToNewGates.find(oldInput.node());
    assert(newInput != oldToNewGates.end());

    newInputs[i] = Gate::Signal(oldInput.event(), newInput->second);
  }

  return newInputs;
}

std::shared_ptr<GNet> PreMapper::map(const GNet &net,
                                     GateIdMap &oldToNewGates) const {
  auto *newNet = mapGates(net, oldToNewGates);

  // Connect the triggers' inputs.
  for (auto oldTriggerId : net.triggers()) {
    const auto *oldTrigger = Gate::get(oldTriggerId);

    auto newTriggerId = oldToNewGates.find(oldTriggerId);
    assert(newTriggerId != oldToNewGates.end());

    auto newInputs = getNewInputs(oldTrigger->inputs(), oldToNewGates);
    newNet->setGate(newTriggerId->second, oldTrigger->func(), newInputs);
  }

  return std::shared_ptr<GNet>(newNet);
}

GNet *PreMapper::mapGates(const GNet &net, GateIdMap &oldToNewGates) const {
  assert(net.isWellFormed() && net.isSorted());

  auto *newNet = new GNet(net.getLevel());

  if (net.isFlat()) {
    for (const auto *oldGate : net.gates()) {
      const auto oldGateId = oldGate->id();
      assert(oldToNewGates.find(oldGateId) == oldToNewGates.end());

      const auto newGateId = mapGate(*oldGate, oldToNewGates, *newNet);
      assert(newGateId != Gate::INVALID);

      oldToNewGates.insert({oldGateId, newGateId});
    }

    return newNet; 
  }

  for (const auto *oldSubnet : net.subnets()) {
    auto *newSubnet = mapGates(*oldSubnet, oldToNewGates);
    newNet->addSubnet(newSubnet);
  }

  return newNet;
}

Gate::Id PreMapper::mapGate(const Gate &oldGate,
                            const GateIdMap &oldToNewGates,
                            GNet &newNet) const {
  if (oldGate.isSource() || oldGate.isTrigger()) {
    // Triggers' inputs will be connected later.
    return newNet.newGate();
  }

  // Just clone the given gate.
  auto newInputs = getNewInputs(oldGate.inputs(), oldToNewGates);
  return newNet.addGate(oldGate.func(), newInputs);
}

} // namespace eda::gate::premapper
