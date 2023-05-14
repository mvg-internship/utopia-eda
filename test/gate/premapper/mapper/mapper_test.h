//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/debugger/checker.h"
#include "gate/premapper/aigmapper.h"
#include "gate/premapper/migmapper.h"
#include "gate/premapper/xagmapper.h"
#include "gate/premapper/xmgmapper.h"

using Gate = eda::gate::model::Gate;
using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
using GateIdMap = std::unordered_map<Gate::Id, Gate::Id>;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;
using Link = Gate::Link;
using PreBasis = eda::gate::premapper::PreBasis;

std::shared_ptr<GNet> makeSingleGateNet(GateSymbol gate, const unsigned N);
std::shared_ptr<GNet> makeSingleGateNetn(GateSymbol gate, const unsigned N);
std::shared_ptr<GNet> makeSingleGateNetOppositeInputs(GateSymbol gate);

std::shared_ptr<GNet> premap(std::shared_ptr<GNet> net,
                             GateIdMap &gmap,
                             PreBasis basis);

bool checkEquivalence(const std::shared_ptr<GNet> net,
                      const std::shared_ptr<GNet> premapped,
                      GateIdMap &gmap);

