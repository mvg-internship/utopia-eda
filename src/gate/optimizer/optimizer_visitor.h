//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/cuts_finder_visitor.h"
#include "gate/optimizer/links_clean.h"
#include "gate/optimizer/rwdatabase.h"
#include "gate/optimizer/util.h"
#include "gate/optimizer/visitor.h"

#include <queue>

namespace eda::gate::optimizer {
/**
 * \brief Realization of interface Visitor.
 * \ Handler of the node and its cut to execute rewriting.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  class OptimizerVisitor : public Visitor {
  public:

    using BoundGNetList = RWDatabase::BoundGNetList;
    using BoundGNet = RWDatabase::BoundGNet;

    OptimizerVisitor();

    void set(CutStorage *cutStorage, GNet *net, int cutSize);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

    virtual bool checkOptimize(const BoundGNet &option,
                               const std::unordered_map<GateID, GateID> &map) = 0;

    virtual void
    considerOptimization(BoundGNet &option,
                         std::unordered_map<GateID, GateID> &map) = 0;

    virtual VisitorFlags finishOptimization() { return SUCCESS; }

    virtual BoundGNetList getSubnets(uint64_t func) = 0;

  private:
    CutStorage *cutStorage;

    CutStorage::Cuts *lastCuts;
    std::vector<const CutStorage::Cut *> toRemove;

    bool checkValidCut(const Cut &cut);

  protected:
    GNet *net;
    GateID lastNode;
    int cutSize;
  };

} // namespace eda::gate::optimizer