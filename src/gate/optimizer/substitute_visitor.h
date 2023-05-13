//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/links_clean.h"
#include "gate/optimizer/visitor.h"
#include "gate/optimizer/walker.h"

namespace eda::gate::optimizer {

  class SubstituteVisitor : public Visitor {
  public:
    using Gate = model::Gate;

    SubstituteVisitor(GateID cutFor, const std::unordered_map<GateID, GateID> &map, GNet *subsNet, GNet *net);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

  private:
    GateID cutFor;
    /// subNetGateId -> sourceNetGaeId
    const std::unordered_map<GateID, GateID> &map;
    GNet *subsNet;
    GNet *net;

    std::unordered_map<GateID, GateID> nodes;

    bool checkOutGate(const Gate *gate) const;
  };

} // namespace eda::gate::optimizer
