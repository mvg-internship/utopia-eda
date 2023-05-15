//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/optimizer.h"
#include "gate/optimizer/tech_map/replacement_struct.h"
#include "gate/optimizer/tech_map/strategy/replacement_cut.h"
#include "gate/optimizer/tech_map/tech_mapper.h"

namespace eda::gate::optimizer {

  void techMap(GNet *net, int cutSize, TechMapVisitor&& techMapper, ReplacementVisitor&& replacer) {
    std::unordered_map<GateID, double> gatesDelay;
    std::unordered_map<GateID, Replacement> bestReplacement;

    CutStorage cutStorage = findCuts(cutSize, net);

    techMapper.set(&cutStorage, net, &bestReplacement, cutSize);

    Walker walker(net, &techMapper, &cutStorage);

    walker.walk(true);

    if (bestReplacement.size() != 0) {
      replacer.set(&cutStorage, net, &bestReplacement, cutSize);

      Walker repl(net, &replacer, &cutStorage);

      repl.walk(false);
    }
  }

} // namespace eda::gate::optimizer
