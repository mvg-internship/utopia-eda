//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "gate/optimizer/visitor.h"

namespace eda::gate::optimizer {
/**
 * \brief Realization of interface Visitor.
 * Removes all gates that get zero fanout after removing links from given node.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  class LinkCleanVisitor : public Visitor {

  public:
    using Gate = eda::gate::model::Gate;
    using GateSymbol = eda::gate::model::GateSymbol;
    using Signal = base::model::Signal<GNet::GateId>;

    LinkCleanVisitor(GateID node, GNet *gNet,
                     const std::vector<Signal> &newSignals);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

  private:
    GateID node;
    const std::vector<Signal> &newSignals;
    GNet *gNet;
  };
} // namespace eda::gate::optimizer