//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once
#include "gate/model/gnet.h"

namespace eda::gate::debugger {
using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;
using GateId = eda::gate::model::Gate::Id;

class LEC {
public:
  virtual bool areEqual(GNet &lhs,
                GNet &rhs,
                std::unordered_map<Gate::Id, Gate::Id> &gmap) = 0;
  virtual ~LEC() = 0;
};

enum LecType {
  BdD,
  DFL,
  RND,
};

LEC &getChecker(LecType lec);

} // namespace eda::gate::debugger
