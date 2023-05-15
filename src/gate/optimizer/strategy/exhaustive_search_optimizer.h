//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/optimizer_visitor.h"
#include "gate/optimizer/rwmanager.h"

namespace eda::gate::optimizer {

  class ExhausitiveSearchOptimizer : public OptimizerVisitor {

  public:
    ExhausitiveSearchOptimizer(const char* namefile) {
      RewriteManager& rewriteManager = RewriteManager::get();
      rewriteManager.initialize(namefile);
      rwdb = rewriteManager.getDatabase(namefile);
    }

    bool checkOptimize(const BoundGNet &option,
                       const std::unordered_map<GateID, GateID> &map) override;

    void considerOptimization(BoundGNet &option,
                              std::unordered_map<GateID, GateID> &map) override;

    BoundGNetList getSubnets(uint64_t func) override;

    VisitorFlags finishOptimization() override;

  private:
    RWDatabase rwdb;
    BoundGNet bestOption;
    std::unordered_map<GateID, GateID> bestOptionMap;
    int bestReduce = 1;

  };
} // namespace eda::gate::optimizer

