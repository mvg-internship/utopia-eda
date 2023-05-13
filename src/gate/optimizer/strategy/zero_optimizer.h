//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/optimizer_visitor.h"

namespace eda::gate::optimizer {

  class ZeroOptimizer : public OptimizerVisitor {
  public:

    bool checkOptimize(const BoundGNet &option,
                       const std::unordered_map<GateID, GateID> &map) override;

    void considerOptimization(BoundGNet &option,
                              std::unordered_map<GateID, GateID> &map) override;

    BoundGNetList getSubnets(uint64_t func) override;
  };
} // namespace eda::gate::optimizer

