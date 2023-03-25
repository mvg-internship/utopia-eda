//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/debugger/checker.h"

namespace eda::gate::debugger {

using Gate = model::Gate;
using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
using GateId = model::Gate::Id;
using GateSymbol = model::GateSymbol;
using GNet = eda::gate::model::GNet;
using Hints = Checker::Hints;
using Signal = model::Gate::Signal;
using SignalList = model::Gate::SignalList; 

/** 
 *  \brief Constructs a miter for equivalence checking.
 *  Hints structure provides correspondence between input and output 
 *  gates of given nets.
 *  @param hints Stores correspondence between gates. 
 *  @return The miter.
 */
GNet* miter(GNet* net1, GNet* net2, Hints &hints);

//Checks if it is possible to construct a miter with given parameters.
bool isMiterable(GNet* net1, GNet* net2, Hints &hints);

} // namespace eda::gate::debugger
