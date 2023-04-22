//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/debugger/checker.h"
#include "util/logging.h"

namespace eda::gate::debugger {

using Gate = model::Gate;
using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
using GateId = model::Gate::Id;
using GateIdList  = std::vector<GateId>;
using GateSymbol = model::GateSymbol;
using GNet = eda::gate::model::GNet;
using Hints = eda::gate::debugger::Checker::Hints;
using Signal = model::Gate::Signal;
using SignalList = model::Gate::SignalList;

/**
 *  \brief Constructs a miter for the specified nets.
 *  @param hints Gate-to-gate mapping between nets.
 *  @return The miter.
 */
GNet *miter(GNet &net1, GNet &net2, Hints &hints);

// Checks if it is possible to construct a miter with given parameters.
bool areMiterable(GNet &net1, GNet &net2, Hints &hints);

} // namespace eda::gate::debugger
