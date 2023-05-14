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
#include "gate/optimizer/tech_map/replacement_struct.h"
#include "gate/optimizer/util.h"
#include "gate/optimizer/visitor.h"

#include <queue>

namespace eda::gate::optimizer {
/**
 * \brief Implementation of interface Visitor.
 * \author <a href="mailto:dgaryaev@ispras.ru"></a>
 */

  class TechMapVisitor : public Visitor {
  public:
    using BoundGNetList = RWDatabase::BoundGNetList;
    using BoundGNet = RWDatabase::BoundGNet;

    TechMapVisitor();

    void set(CutStorage *cutStorage, GNet *net,
        std::unordered_map<GateID, Replacement> *bestReplacement,
        int cutSize);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;

    std::unordered_map<GateID, Replacement> *bestReplacement;

  private:
    CutStorage *cutStorage;
    //std::unordered_map<GateID, double> *gatesDelay;

    CutStorage::Cuts *lastCuts;
    std::vector<const CutStorage::Cut*> toRemove;

    bool checkValidCut(const Cut &cut);

  protected:
    GNet *net;
    GateID lastNode;
    int cutSize;

    virtual bool checkOptimize(const BoundGNet &superGate,
        const std::unordered_map<GateID, GateID> &map) = 0;

    virtual VisitorFlags
    considerTechMap(BoundGNet &superGate,
        std::unordered_map<GateID, GateID> &map) = 0;

    virtual void finishTechMap() {}

    virtual BoundGNetList getSubnets(uint64_t func) = 0;
  };

} // namespace eda::gate::optimizer
