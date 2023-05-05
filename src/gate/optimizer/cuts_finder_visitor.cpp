//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/cuts_finder_visitor.h"

namespace eda::gate::optimizer {

  using Gate = eda::gate::model::Gate;
  using Cuts = CutStorage::Cuts;
  using Cut = CutStorage::Cut;
  using CutIt = Cuts::iterator;

  CutsFindVisitor::CutsFindVisitor(int cutSize, CutStorage *cutStorage) :
          cutSize(cutSize), cutStorage(cutStorage) {}

  VisitorFlags CutsFindVisitor::onCut(const Cut &cut) {
    return VisitorFlags::FINISH_THIS;
  }

  VisitorFlags CutsFindVisitor::onNodeBegin(const GateID &vertex) {
    Gate *gate = Gate::get(vertex);
    auto *cuts = &cutStorage->cuts[vertex];

    // Adding trivial cut.
    Cut self;
    self.emplace(vertex);
    cuts->emplace(self);

    std::vector<CutIt> ptrs;
    std::vector<Cuts *> inputCuts;

    ptrs.reserve(gate->inputs().size());
    inputCuts.reserve(gate->inputs().size());

    // Initializing ptrs to cuts of input.
    for (auto input: gate->inputs()) {
      inputCuts.push_back(&cutStorage->cuts[input.node()]);
      ptrs.push_back(inputCuts.back()->begin());
    }

    size_t i = 0;
    while (true) {
      // Fix cut.
      Cut collected;

      for (auto &it: ptrs) {
        collected.insert((*it).begin(), (*it).end());
        if (static_cast<int>(collected.size()) > cutSize) {
          collected = Cut();
          break;
        }
      }

      // Saving cut if the iteration produced good size cut.
      if (!collected.empty()) {
        cuts->emplace(collected);
        if(cuts->size() > 100) {
          return VisitorFlags::SUCCESS;
        }
      }

      // Incrementing iterators to move to the next combination of nodes for cut.
      size_t prevInd = i;
      while (i < inputCuts.size() && ++ptrs[i] == inputCuts[i]->end()) {
        ptrs[i] = inputCuts[i]->begin();
        ++i;
      }

      if (i >= inputCuts.size()) {
        break;
      }

      if (i != prevInd) {
        i = 0;
      }
    }
    return VisitorFlags::SUCCESS;
  }

  VisitorFlags CutsFindVisitor::onNodeEnd(const GateID &) {
    return VisitorFlags::SUCCESS;
  }
} // namespace eda::gate::optimizer
