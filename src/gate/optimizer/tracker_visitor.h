//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/visitor.h"
#include "gate/printer/dot.h"

#include <filesystem>

namespace eda::gate::optimizer {
  class TrackerVisitor : public Visitor {
  public:

    TrackerVisitor(const std::filesystem::path &subCatalog, const GNet *net,
                   Visitor *visitor);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

  private:
    std::filesystem::path subCatalog;
    Visitor *visitor;
    Dot dot;
    int counter = 0;
  };

} // namespace eda::gate::optimizer
