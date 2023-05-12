//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "tracker_visitor.h"

namespace eda::gate::optimizer {

  TrackerVisitor::TrackerVisitor(const std::filesystem::path &subCatalog,
                                 const GNet *net,
                                 Visitor *visitor) : visitor(visitor),
                                                     dot(net) {

    const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
    this->subCatalog = homePath / subCatalog;
  }

  VisitorFlags TrackerVisitor::onNodeBegin(const GateID &gateId) {
    return visitor->onNodeBegin(gateId);
  }

  VisitorFlags TrackerVisitor::onNodeEnd(const GateID &gateId) {
    dot.print(subCatalog / ("onNodeEnd" + std::to_string(counter) + "_" +
                            std::to_string(gateId) + ".dot"));
    ++counter;
    return visitor->onNodeEnd(gateId);
  }

  VisitorFlags TrackerVisitor::onCut(const Cut &cut) {
    return visitor->onCut(cut);
  }

} // namespace eda::gate::optimizer
