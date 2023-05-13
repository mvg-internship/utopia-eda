//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "gate/optimizer/cone_visitor.h"
#include "gate/optimizer/links_add_counter.h"
#include "gate/optimizer/links_clean.h"
#include "gate/optimizer/links_clean_counter.h"
#include "gate/optimizer/substitute_visitor.h"
#include "gate/optimizer/ttbuilder.h"
#include "gate/optimizer/walker.h"

/**
 * \brief Methods used for rewriting.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
namespace eda::gate::optimizer {

  using GNet = model::GNet;
  using GateID = model::GNet::GateId;
  using Gate = model::Gate;
  using Cut = CutStorage::Cut;

  void substitute(GateID cutFor, const std::unordered_map<GateID, GateID> &map, GNet *subsNet, GNet *net);

  int fakeSubstitute(GateID cutFor, const std::unordered_map<GateID, GateID> &map, GNet *subsNet, GNet *net);

} // namespace eda::gate::optimizer

