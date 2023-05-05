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

  class LinkAddCounter : public Visitor {
  public:
    using Gate = eda::gate::model::Gate;
    using Signal = base::model::Signal<GNet::GateId>;

    LinkAddCounter(GNet *net, const std::unordered_map<GateID, GateID> &map);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

    const std::unordered_set<GateID> *getUsed() { return &used; };

    int getNAdded() const { return added; };

    bool checkOutGate(const Gate *gate) const;

  private:
    GNet *net;
    const std::unordered_map<GateID, GateID> &map;
    std::unordered_map<GateID, GateID> substitute;
    std::unordered_set<GateID> used;
    int added = 0;
  };
} // namespace eda::gate::optimizer
