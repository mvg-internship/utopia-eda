//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/util.h"

namespace eda::gate::optimizer {

  void substitute(GateID cutFor, const std::unordered_map<GateID, GateID> &map, GNet *subsNet, GNet *net) {
    SubstituteVisitor visitor(cutFor, map, subsNet, net);
    Walker walker(subsNet, &visitor, nullptr);
    walker.walk(true);
  }

  int fakeSubstitute(GateID cutFor, const std::unordered_map<GateID, GateID> &map, GNet *subsNet, GNet *net) {

    LinkAddCounter addCounter(net, map);
    Walker walker(subsNet, &addCounter, nullptr);
    walker.walk(true);

    LinksRemoveCounter removeCounter(cutFor, addCounter.getUsed());
    walker = Walker(net, &removeCounter, nullptr);
    // TODO Change to found earlier cone.
    walker.walk(cutFor, false);
    return addCounter.getNAdded() - removeCounter.getNRemoved();
  }

} // namespace eda::gate::optimizer