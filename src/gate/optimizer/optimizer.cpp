//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/optimizer.h"

namespace eda::gate::optimizer {

  void optimize(GNet *net, int cutSize, OptimizerVisitor &&optimizer) {
    CutStorage cutStorage = findCuts(cutSize, net);

    std::cout << "cuts found" << std::endl;

    optimizer.set(&cutStorage, net, cutSize);
    Walker walker(net, &optimizer, &cutStorage);

    // TODO: change for normal condition.
    while (true) {
      walker.walk(true);
      break;
    }
  }

  void
  optimizePrint(GNet *net, int cutSize, const std::filesystem::path &subCatalog,
                OptimizerVisitor &&optimizer) {
    CutStorage cutStorage = findCuts(cutSize, net);

    std::cout << "cuts found " << std::endl;

    optimizer.set(&cutStorage, net, cutSize);
    TrackerVisitor trackerVisitor(subCatalog, net, &optimizer);
    Walker walker(net, &trackerVisitor, &cutStorage);

    // TODO: change for normal condition.
    while (true) {
      walker.walk(true);
      break;
    }
  }

  void optimizeTrackPrint(GNet *net, int cutSize,
                          const std::filesystem::path &subCatalog,
                          OptimizerVisitor &&optimizer) {
    CutStorage cutStorage = findCuts(cutSize, net);

    std::cout << "cuts found " << std::endl;

    optimizer.set(&cutStorage, net, cutSize);
    TrackStrategy trackStrategy(subCatalog, &optimizer);
    trackStrategy.set(&cutStorage, net, cutSize);

    TrackerVisitor trackerVisitor(subCatalog, net, &trackStrategy);
    Walker walker(net, &trackerVisitor, &cutStorage);

    // TODO: change for normal condition.
    while (true) {
      walker.walk(true);
      break;
    }
  }

  CutStorage findCuts(int cutSize, GNet *net) {
    CutStorage cutStorage;

    CutsFindVisitor visitor(cutSize, &cutStorage);
    Walker firstFind(net, &visitor, &cutStorage);
    // Find cuts on the first iteration.
    firstFind.walk(true);

    return cutStorage;
  }

} // namespace eda::gate::optimizer