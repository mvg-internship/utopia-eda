//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once
#include "base_checker.h"
#include "gate/debugger/miter.h"
#include "gate/transformer/bdd.h"

namespace eda::gate::debugger {

using BddList = transformer::GNetBDDConverter::BDDList;
using Gate = model::Gate;
using GateBDDMap = transformer::GNetBDDConverter::GateBDDMap;
using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
using GateId = model::Gate::Id;
using GateList = transformer::GNetBDDConverter::GateList;
using GateSymbol = model::GateSymbol;
using GateUintMap = transformer::GNetBDDConverter::GateUintMap;
using GNet = eda::gate::model::GNet;
using GNetBDDConverter = transformer::GNetBDDConverter;
using Hints = Checker::Hints;
using Signal = model::Gate::Signal;
using SignalList = model::Gate::SignalList;

/** 
 *  \brief Checks the equivalence of the specified nets using BDD converter.
 *  @param hints Gate-to-gate mapping between nets. 
 *  @return true if the nets are equivalent, false if not.
 */
bool bddChecker(GNet &net1, GNet &net2, Hints &hints);

class BddChecker : public BaseChecker, public util::Singleton<BddChecker> {
friend class util::Singleton<BddChecker>;

public:
  bool areEqual(GNet &lhs,
                GNet &rhs,
                Checker::GateIdMap &gmap) override;

};

} // namespace eda::gate::debugger
