//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "gate/optimizer/cut_storage.h"

#include <queue>

namespace eda::gate::optimizer {

  using GateID = eda::gate::model::GNet::GateId;
  using Gate = eda::gate::model::Gate;
  using Cut = CutStorage::Cut;

  /**
 * \brief Checking that a given cut is indeed a cut for a given vertex x.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  bool isCut(const GateID &gate, const Cut &cut, GateID &failed);

} // namespace eda::gate::optimizer
