//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/visitor.h"

namespace eda::gate::optimizer {
  class LinksRemoveCounter : public Visitor {

  public:
    using Gate = eda::gate::model::Gate;
    using GateSymbol = eda::gate::model::GateSymbol;
    using Signal = base::model::Signal<GNet::GateId>;

    LinksRemoveCounter(GateID node,
                       const std::unordered_set<GateID> *used);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

    int getNRemoved() { return static_cast<int>(removed.size() - 1); }

  private:
    GateID node;
    const std::unordered_set<GateID> *used;
    std::unordered_set<GateID> removed;
  };
} // namespace eda::gate::optimizer

