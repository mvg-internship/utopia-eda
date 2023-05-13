//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/links_add_counter.h"

namespace eda::gate::optimizer {

  LinkAddCounter::LinkAddCounter(GNet *net,
                                 const std::unordered_map<GateID, GateID> &map)
          : net(net), map(map) {}

  VisitorFlags LinkAddCounter::onNodeBegin(const GateID &id) {
    Gate *gate = Gate::get(id);
    // Handling sources.
    if (gate->isSource()) {

      auto found = map.find(id);
      if (found != map.end()) {
        substitute[id] = found->second;
        used.emplace(found->second);
      } else {
        // TODO: implement strategy here.
        substitute[id] = Gate::INVALID;
        ++added;
      }

      // Handling out gate.
    } else if (checkOutGate(gate)) {
      return FINISH_ALL;
    } else if (!gate->links().empty()) {
      std::vector<Signal> signals;
      // Mapping signals.
      for (const auto &input: gate->inputs()) {
        auto found = substitute.find(input.node());
        if (found != substitute.end()) {
          signals.emplace_back(input.event(), found->second);
        } else {
          ++added;
          return SUCCESS;
        }
      }
      const auto *subGate = net->gate(gate->func(), signals);
      if (subGate) {
        used.emplace(subGate->id());
        substitute[id] = subGate->id();
      } else {
        ++added;
      }
    }
    return SUCCESS;
  }

  VisitorFlags LinkAddCounter::onNodeEnd(const Visitor::GateID &) {
    return SUCCESS;
  }

  VisitorFlags LinkAddCounter::onCut(const Cut &) {
    return SUCCESS;
  }

  bool LinkAddCounter::checkOutGate(const Gate *gate) const {
    // Output gate has either more than 2 inputs
    // or net consists only of inputs and targets.
    assert(!gate->isTarget() && "Rewriting input net error.");

    if (gate->links().empty()) {
      return true;
    }
    if (gate->links().size() == 1) {
      Gate *next = Gate::get(gate->links().front().target);
      return next->isTarget();
    }
    return false;
  }

} // namespace eda::gate::optimizer
