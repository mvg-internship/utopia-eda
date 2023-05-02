//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/utils.h"
#include "gate/premapper/aigmapper.h"
#include "gate/premapper/migmapper.h"
#include "gate/premapper/premapper.h"
#include "gate/premapper/xagmapper.h"
#include "gate/premapper/xmgmapper.h"

#include <cassert>
#include <unordered_map>

namespace eda::gate::premapper {

using Gate = eda::gate::model::Gate;
using GNet = eda::gate::model::GNet;
using SignalList = model::Gate::SignalList;

PreMapper &getPreMapper(const PreBasis basis) {
  switch(basis) {
  case PreBasis::MIG: return MigMapper::get();
  case PreBasis::XAG: return XagMapper::Singleton<XagMapper>::get();
  case PreBasis::XMG: return XmgMapper::Singleton<XmgMapper>::get();
  case PreBasis::AIG: return AigMapper::get();
  default: return AigMapper::get();
  }
}

std::shared_ptr<GNet> PreMapper::map(const GNet &net,
                                     GateIdMap &oldToNewGates) const {
  auto *newNet = mapGates(net, oldToNewGates);

  // Connect the triggers' inputs.
  for (const auto oldTriggerId : net.triggers()) {
    const auto *oldTrigger = Gate::get(oldTriggerId);

    auto newTriggerId = oldToNewGates.find(oldTriggerId);
    assert((newTriggerId != oldToNewGates.end()) && "Invalid new trigger ID");

    auto newInputs = model::getNewInputs(oldTrigger->inputs(), oldToNewGates);
    newNet->setGate(newTriggerId->second, oldTrigger->func(), newInputs);
  }

  return std::shared_ptr<GNet>(newNet);
}

GNet *PreMapper::mapGates(const GNet &net,
                          GateIdMap &oldToNewGates) const {
  assert((net.isWellFormed() && net.isSorted()) &&
         "Orphans, empty subnets, network is not flat or sorted");

  auto *newNet = new GNet(net.getLevel());

  if (net.isFlat()) {
    for (const auto *oldGate : net.gates()) {
      const auto oldGateId = oldGate->id();
      assert((oldToNewGates.find(oldGateId) == oldToNewGates.end()) &&
             "Invalid new gate ID");

      const auto newGateId = mapGate(*oldGate, oldToNewGates, *newNet);
      assert((newGateId != Gate::INVALID) && "Invalid gate used");

      oldToNewGates.emplace(oldGateId, newGateId);
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
  auto newInputs = model::getNewInputs(oldGate.inputs(), oldToNewGates);
  return newNet.addGate(oldGate.func(), newInputs);
}

} // namespace eda::gate::premapper