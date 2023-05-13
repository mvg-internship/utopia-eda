//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/links_clean_counter.h"

namespace eda::gate::optimizer {

  LinksRemoveCounter::LinksRemoveCounter(Visitor::GateID node,
                                         const std::unordered_set<GateID> *used)
          : node(node), used(used) {
    removed.emplace(node);
  }

  VisitorFlags LinksRemoveCounter::onNodeBegin(const Visitor::GateID &node) {
    if (this->node == node) {
      return SUCCESS;
    }

    if (used->find(node) != used->end()) {
      return FINISH_THIS;
    }

    const auto &links = Gate::get(node)->links();
    for (const auto &link: links) {
      if (removed.find(link.target) == removed.end()) {
        return FINISH_THIS;
      }
    }
    removed.emplace(node);

    return SUCCESS;
  }

  VisitorFlags LinksRemoveCounter::onNodeEnd(const Visitor::GateID &) {
    return SUCCESS;
  }

  VisitorFlags LinksRemoveCounter::onCut(const Cut &cut) {
    return SUCCESS;
  }
} // namespace eda::gate::optimizer