//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/tech_map/strategy/simple_techmapper.h"

namespace eda::gate::optimizer {
  using BoundGNetList = RWDatabase::BoundGNetList;

  bool SimpleTechMapper::checkOptimize(const BoundGNet &superGate,
      const std::unordered_map<GateID, GateID> &map) {
    double maxGateArrivalTime = maxArrivalTime(superGate, map);
    if ( minNodeArrivalTime > maxGateArrivalTime) {
      minNodeArrivalTime = maxGateArrivalTime;
      return true;
    }
    return false;
  }

  VisitorFlags
  SimpleTechMapper::considerTechMap(BoundGNet &superGate,
      std::unordered_map<GateID, GateID> &map) {
    bestOption = std::move(superGate);
    bestOptionMap = std::move(map);
    return SUCCESS;
  }

  BoundGNetList
  SimpleTechMapper::getSubnets(uint64_t func) {
    return rwdb.get(func);
  }

  void SimpleTechMapper::finishTechMap() {
    if (bestOptionMap.size() != 0) {
      Replacement bestReplacment{lastNode, bestOptionMap, bestOption.net.get(),
          net, minNodeArrivalTime};
      bestReplacement->insert(std::pair<GateID, Replacement>
          (lastNode, bestReplacment));
    }
  }

  double SimpleTechMapper::maxArrivalTime(const BoundGNet &superGate,
      const std::unordered_map<GateID, GateID> &map) {
    double maxDelay = 0;
    std::unordered_map<Gate::Id, uint32_t> revGareBindings;

    for (const auto &[superInputId, superGateId] : superGate.bindings) {
      revGareBindings[superGateId] = superInputId;
    }

    for (const auto &[inputId, gateId] : map) {
      double delay = 0;

      if (bestReplacement->count(gateId)) {
        delay = bestReplacement->at(gateId).delay;
      }
      delay = delay + superGate.inputsDelay.at(revGareBindings.at(inputId));

      if (delay > maxDelay) {
        maxDelay = delay;
      }
    }
    return maxDelay;
  }

} // namespace eda::gate::optimizer
