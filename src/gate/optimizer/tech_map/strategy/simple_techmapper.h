//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/rwmanager.h"
#include "gate/optimizer/tech_map/tech_map_visitor.h"

#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

namespace eda::gate::optimizer {

  class SimpleTechMapper : public TechMapVisitor {
  public:
    SimpleTechMapper(const char* namefile) {
      RewriteManager& rewriteManager = RewriteManager::get();
      rewriteManager.initialize(namefile);
      rwdb = rewriteManager.getDatabase(namefile);
    }

  protected:
    RWDatabase rwdb;

    double minNodeArrivalTime = std::numeric_limits<double>::max();

    BoundGNet bestOption;
    std::unordered_map<GateID, GateID> bestOptionMap;

    bool checkOptimize(const BoundGNet &superGate,
        const std::unordered_map<GateID, GateID> &map) override;

    VisitorFlags considerTechMap(BoundGNet &superGate,
        std::unordered_map<GateID, GateID> &map) override;

    BoundGNetList getSubnets(uint64_t func) override;

    void finishTechMap() override;

    double maxArrivalTime(const BoundGNet &superGate,
        const std::unordered_map<GateID, GateID> &map);
  };

} // namespace eda::gate::optimizer
