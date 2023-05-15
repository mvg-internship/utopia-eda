//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//


#pragma once

#include "gate/model/gnet.h"
#include "gate/optimizer/rwdatabase.h"

#include <unordered_map>

namespace eda::gate::optimizer {
/**
 * \brief Struct for descriptions Supper Gate
 * \author <a href="mailto:dgaryaev@ispras.ru"></a>
 */
  struct Replacement {
    using GNet = model::GNet;
    using GateID = GNet::GateId;
    using BoundGNet = RWDatabase::BoundGNet;

    GateID rootNode;
    std::unordered_map<GateID, GateID> bestOptionMap;
    GNet *subsNet;
    GNet *net;
    double delay;
  };
} // namespace eda::gate::optimizer
