//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/cut_storage.h"
#include "gate/optimizer/visitor.h"

#include <deque>

namespace eda::gate::optimizer {
/**
 * \brief Realization of interface Visitor.
 * \ Finds cone for given node and its cut.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  class ConeVisitor : public Visitor {
  public:

    using Gate = eda::gate::model::Gate;
    using GateSymbol = eda::gate::model::GateSymbol;

    explicit ConeVisitor(const Cut &cut);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

    GNet *getGNet();

    const std::unordered_map<GateID, GateID> &getResultCut() const {
      return resultCut;
    }

  private:
    const Cut &cut;
    std::deque<GateID> visited;
    std::unordered_map<GateID, GateID> newGates;
    std::unordered_map<GateID, GateID> resultCut;
  };
} // namespace eda::gate::optimizer
