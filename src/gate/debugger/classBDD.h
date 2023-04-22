//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once
#include "bdd_checker.h"
#include "classLEC.h"
#include "miter.h"

namespace eda::gate::debugger {

class BDDChecker : public LEC, public util::Singleton<BDDChecker> {
friend class util::Singleton<BDDChecker>;

public:
  bool areEqual(GNet &lhs,
                GNet &rhs,
                Checker::GateIdMap &gmap) override;

};
} // namespace eda::gate::debugger
