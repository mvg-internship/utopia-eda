//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "exhaustive_search_optimizer.h"

namespace eda::gate::optimizer {
  using BoundGNetList = RWDatabase::BoundGNetList;

  bool ExhausitiveSearchOptimizer::checkOptimize(const BoundGNet &option,
                                                 const std::unordered_map<GateID, GateID> &map) {
    int reduce = fakeSubstitute(lastNode, map, option.net.get(), net);
    if (reduce < bestReduce) {
      bestReduce = reduce;
      return true;
    }
    return false;
  }

  void
  ExhausitiveSearchOptimizer::considerOptimization(BoundGNet &option,
                                                   std::unordered_map<GateID, GateID> &map) {
    bestOption = std::move(option);
    bestOptionMap = std::move(map);
  }

  BoundGNetList
  ExhausitiveSearchOptimizer::getSubnets(uint64_t func) {
    return rwdb.get(func);
  }

  VisitorFlags ExhausitiveSearchOptimizer::finishOptimization() {
    if(bestReduce <= 0) {
      substitute(lastNode, bestOptionMap, bestOption.net.get(), net);
    }
    bestReduce = 1;
    return SUCCESS;
  }

} // namespace eda::gate::optimizer
