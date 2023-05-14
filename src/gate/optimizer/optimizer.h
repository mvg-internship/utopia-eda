//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/cuts_finder_visitor.h"
#include "gate/optimizer/links_clean.h"
#include "gate/optimizer/optimizer_visitor.h"
#include "gate/optimizer/strategy/track_strategy.h"
#include "gate/optimizer/tracker_visitor.h"
#include "gate/optimizer/walker.h"

#include <queue>

namespace eda::gate::optimizer {

  using GNet = eda::gate::model::GNet;
  using GateID = eda::gate::model::GNet::GateId;
  using Gate = eda::gate::model::Gate;
  using Cut = std::unordered_set<GateID>;

  void optimize(GNet *net, int cutSize, OptimizerVisitor &&optimizer);

  void
  optimizePrint(GNet *net, int cutSize, const std::filesystem::path &subCatalog,
                OptimizerVisitor &&optimizer);

  void optimizeTrackPrint(GNet *net, int cutSize,
                          const std::filesystem::path &subCatalog,
                          OptimizerVisitor &&optimizer);

  CutStorage findCuts(int cutSize, GNet *net);
} // namespace eda::gate::optimizer
