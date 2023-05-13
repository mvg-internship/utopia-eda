//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "gate/optimizer/cut_storage.h"
#include "gate/optimizer/visitor.h"
#include "util/graph.h"

#include <queue>

namespace eda::gate::optimizer {
/**
 * \brief Class traces nodes in topological order.
 * \ Calls visitor to handle each node or cut.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  class Walker {
    using GNet = eda::gate::model::GNet;
    using Gate = eda::gate::model::Gate;
    using GateID = GNet::GateId;
    using Cut = CutStorage::Cut;

    GNet *gNet;
    Visitor *visitor;
    CutStorage *cutStorage;

    bool checkVisited(std::unordered_set<GateID> &visited, GateID node,
                      bool forward);

    std::vector<GateID> getNext(GateID node, bool forward);


    VisitorFlags callVisitor(GateID node);

  public:
    Walker(GNet *gNet, Visitor *visitor, CutStorage *cutStorage);

    void walk(bool forward);

    void walk(GateID start, const Cut &cut, bool forward);

    void walk(GateID start, bool forward);
  };
} // namespace eda::gate::optimizer
