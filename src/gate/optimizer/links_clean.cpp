//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/links_clean.h"

namespace eda::gate::optimizer {

  LinkCleanVisitor::LinkCleanVisitor(GateID node, GNet *gNet,
                                     const std::vector<eda::base::model::Signal<GNet::GateId>> &newSignals)
          : node(node), newSignals(newSignals),
            gNet(gNet) {}

  VisitorFlags LinkCleanVisitor::onNodeBegin(const GateID &node) {
    if (this->node == node) {
        gNet->setGate(node, Gate::get(node)->func(), newSignals);
      }  else if (Gate::get(node)->fanout() == 0) {
      gNet->eraseGate(node);
    }
    /*
     * else {
        Gate *oldGate = Gate::get(oldOut);
        Gate *newGate = Gate::get(newOut);
        // Removing links from outputs of oldGate and
        // connecting links of old gate to the new gate.
        for(const auto & out : oldGate->links()) {
          Gate *outGate = Gate::get(out.target);
          auto inputs = outGate->inputs();
          auto it = std::find(inputs.begin(), inputs.end(), oldOut);
          *it = newOut;
          gNet->setGate(out.target, outGate->func(), inputs);
        }
      }
    }
     */
    return SUCCESS;
  }

  VisitorFlags LinkCleanVisitor::onNodeEnd(const GateID &) {
    return SUCCESS;
  }

  VisitorFlags LinkCleanVisitor::onCut(const Cut &) {
    return SUCCESS;
  }
} // namespace eda::gate::optimizer
