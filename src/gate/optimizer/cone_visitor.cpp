//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/cone_visitor.h"

namespace eda::gate::optimizer {

  ConeVisitor::ConeVisitor(const Cut &cut) : cut(cut) {
  }

  VisitorFlags ConeVisitor::onNodeBegin(const GateID &node) {
    visited.push_back(node);
    if (cut.find(node) != cut.end()) {
      resultCut.emplace(node, Gate::INVALID);
      return FINISH_THIS;
    }
    return SUCCESS;
  }

  VisitorFlags ConeVisitor::onNodeEnd(const GateID &) {
    return SUCCESS;
  }

  VisitorFlags ConeVisitor::onCut(const Cut &) {
    return SUCCESS;
  }

  ConeVisitor::GNet *ConeVisitor::getGNet() {
    GNet *coneNet = new GNet();

    GateID first = visited.front();

    while (!visited.empty()) {

      auto cur = visited.back();
      std::vector<base::model::Signal<GateID>> signals;
      const Gate *curGate = Gate::get(cur);
      const auto &inputs = curGate->inputs();
      visited.pop_back();

      for (const auto &signal: inputs) {
        auto found = newGates.find(signal.node());
        if (found != newGates.end()) {
          signals.emplace_back(base::model::Event::ALWAYS, found->second);
        }
      }

      if(resultCut.find(cur) != resultCut.end()) {
        if(signals.empty()) {
          resultCut[cur] = newGates[cur] = coneNet->addGate(GateSymbol::IN);
        } else {
          resultCut[cur] = newGates[cur] = coneNet->addGate(curGate->func(),signals);
        }
      } else {
        if(signals.empty()) {
          resultCut[cur] = newGates[cur] = coneNet->addGate(GateSymbol::IN);
        } else {
          newGates[cur] = coneNet->addGate(curGate->func(), signals);
        }
      }
    }

    // Adding target gate.
    if(Gate::get(first)->func() != GateSymbol::OUT) {
      coneNet->addGate(GateSymbol::OUT, newGates[first]);
    }

    return coneNet;
  }
} // namespace eda::gate::optimizer