//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

namespace eda::gate::debugger::options {

enum LecType {
  BDD,
  DEFAULT,
  RND,
};

} // namespace eda::gate::debugger::options

namespace eda::gate::debugger {
using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;
using GateId = eda::gate::model::Gate::Id;
using LecType = eda::gate::debugger::options::LecType;

class BaseChecker {
public:
  virtual bool areEqual(GNet &lhs,
                        GNet &rhs,
                        std::unordered_map<Gate::Id, Gate::Id> &gmap) = 0;
  virtual ~BaseChecker() = 0;
};

BaseChecker &getChecker(LecType lec);

} // namespace eda::gate::debugger
