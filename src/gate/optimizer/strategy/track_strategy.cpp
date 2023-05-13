//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/strategy/track_strategy.h"

namespace eda::gate::optimizer {

  TrackStrategy::TrackStrategy(const std::filesystem::path &subCatalog,
                               OptimizerVisitor *visitor) : visitor(visitor) {
    const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
    this->subCatalog = homePath / subCatalog;
  }

  bool TrackStrategy::checkOptimize(const OptimizerVisitor::BoundGNet &option,
                                    const std::unordered_map<GateID, GateID> &map) {

    bool result = visitor->checkOptimize(option, map);

    Dot dot(option.net.get());
    dot.print(subCatalog / ("checkOptimize" + std::to_string(counter) + "_" +
                            std::to_string(lastNode) + "_" +
                            std::to_string(result) + ".dot"));
    ++counter;

    return result;
  }

  void
  TrackStrategy::considerOptimization(OptimizerVisitor::BoundGNet &option,
                                      std::unordered_map<GateID, GateID> &map) {
    visitor->considerOptimization(option, map);
  }

  OptimizerVisitor::BoundGNetList TrackStrategy::getSubnets(uint64_t func) {
    auto list = visitor->getSubnets(func);
    std::cout << "for node " << lastNode << " obtained number of gates "
              << list.size() << std::endl;
    return list;
  }

  VisitorFlags TrackStrategy::finishOptimization() {
    return visitor->finishOptimization();
  }

  VisitorFlags TrackStrategy::onNodeBegin(const GateID &id) {
    visitor->onNodeBegin(id);
    return OptimizerVisitor::onNodeBegin(id);
  }

} // namespace eda::gate::optimizer
