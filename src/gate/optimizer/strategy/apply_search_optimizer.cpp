//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "apply_search_optimizer.h"

namespace eda::gate::optimizer {
  using BoundGNetList = RWDatabase::BoundGNetList;

  bool ApplySearchOptimizer::checkOptimize(const BoundGNet &option,
                                                 const std::unordered_map<GateID, GateID> &map) {
    return fakeSubstitute(lastNode, map, option.net.get(), net) <= 0;
  }

  void
  ApplySearchOptimizer::considerOptimization(BoundGNet &option,
                                             std::unordered_map<GateID, GateID> &map) {
    substitute(lastNode, map, option.net.get(), net);
  }

  BoundGNetList
  ApplySearchOptimizer::getSubnets(uint64_t func) {
    return rwdb.get(func);
  }
} // namespace eda::gate::optimizer
