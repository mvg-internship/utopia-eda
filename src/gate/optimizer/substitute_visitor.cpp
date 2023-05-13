//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/substitute_visitor.h"

namespace eda::gate::optimizer {

  SubstituteVisitor::SubstituteVisitor(GateID cutFor,
                                       const std::unordered_map<GateID, GateID> &map,
                                       GNet *subsNet, GNet *net) :
          cutFor(cutFor), map(map), subsNet(subsNet), net(net) {

  }

  VisitorFlags SubstituteVisitor::onNodeBegin(const GateID &gateId) {
    Gate *subGate = Gate::get(gateId);

    // Handling sources of the net.
    if (subGate->isSource()) {
      // TODO: insert map
      auto found = map.find(gateId);
      if (found != map.end()) {
        nodes[gateId] = found->second;
      } else {
        // TODO: implement strategy here.
        nodes[gateId] = net->newGate();
      }
    } else {
      std::vector<eda::base::model::Signal<GNet::GateId>> signals;

      for (const auto &input: subGate->inputs()) {
        signals.emplace_back(input.event(), nodes[input.node()]);
      }

      // Handling output gate.
      if (checkOutGate(subGate)) {
        GateID changeGate;
        Gate *cutForGate = Gate::get(cutFor);
        if (cutForGate->isTarget()) {
          if(signals.size() > 1) {
            changeGate = nodes[subGate->id()] = net->addGate(subGate->func(),
                                                             signals);
            signals = {changeGate};
            nodes[subGate->id()] = changeGate;
          }
          changeGate = nodes[subGate->id()] = cutFor;
        } else {
          changeGate = nodes[subGate->id()] =  cutFor;
          if(cutForGate->func() != subGate->func()) {
            net->setGate(cutFor, subGate->func(), Gate::get(cutFor)->inputs());
          }
        }
        // If input links coincide then don't rewrite anything.
        if (Gate::get(changeGate)->inputs() == signals) {
          return FINISH_ALL;
        }
        // Deleting links.
        LinkCleanVisitor visitor(changeGate, net, signals);
        Walker walker(net, &visitor, nullptr);
        // TODO Change to found earlier cone.
        walker.walk(cutFor, false);
        return FINISH_ALL;
      } else {
        nodes[subGate->id()] = net->addGate(subGate->func(), signals);
      }
    }
    return SUCCESS;
  }

  VisitorFlags SubstituteVisitor::onNodeEnd(const Visitor::GateID &) {
    return SUCCESS;
  }

  VisitorFlags SubstituteVisitor::onCut(const Cut &) {
    return SUCCESS;
  }

  bool SubstituteVisitor::checkOutGate(const Gate *gate) const {
    // Output gate has either more than 2 inputs
    // or net consists only of inputs and targets.
    assert(!gate->isTarget() && "Rewriting input net error.");

    if (gate->links().empty()) {
      assert(nodes.size() + 1 == subsNet->nGates()
             &&
             "There is probably more than one out gates in the substitute net.");
      return true;
    }
    if (gate->links().size() == 1) {
      Gate *next = Gate::get(gate->links().front().target);
      if (next->isTarget()) {
        assert(nodes.size() + 2 == subsNet->nGates() &&
               "Out gate has more than one input.");
        return true;
      }
    }
    return false;
  }

}// namespace eda::gate::optimizer
