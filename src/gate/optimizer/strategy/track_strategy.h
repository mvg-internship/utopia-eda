//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/optimizer_visitor.h"
#include "gate/printer/dot.h"

#include <filesystem>

namespace eda::gate::optimizer {
  class TrackStrategy : public OptimizerVisitor {

  public:

    VisitorFlags onNodeBegin(const GateID &) override;

    TrackStrategy(const std::filesystem::path &subCatalog,
                  OptimizerVisitor *visitor);

    bool checkOptimize(const BoundGNet &option,
                       const std::unordered_map<GateID, GateID> &map) override;

    void considerOptimization(BoundGNet &option,
                              std::unordered_map<GateID, GateID> &map) override;

    BoundGNetList getSubnets(uint64_t func) override;

    VisitorFlags finishOptimization() override;

  private:
    std::filesystem::path subCatalog;
    OptimizerVisitor *visitor;
    int counter = 0;
  };
} // namespace eda::gate::optimizer

