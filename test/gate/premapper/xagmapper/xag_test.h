//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/premapper/xagmapper.h"


using Gate = eda::gate::model::Gate;
using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
using GateIdMap = std::unordered_map<Gate::Id, Gate::Id>;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;
using Link = Gate::Link;

void dump(const GNet &net);

std::shared_ptr<GNet> makeSingleGateNet(GateSymbol gate, const unsigned N);
std::shared_ptr<GNet> makeSingleGateNetn(GateSymbol gate, const unsigned N);
std::shared_ptr<GNet> premap(std::shared_ptr<GNet> net, GateIdMap &gmap);

void initializeBinds(const GNet &net,
                     GateIdMap &gmap,
                     GateBinding &ibind,
                     GateBinding &obind);
bool checkEquivalence(const std::shared_ptr<GNet> net,
                      const std::shared_ptr<GNet> premapped,
                      GateIdMap &gmap);
