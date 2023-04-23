//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once
#include "base_checker.h"
#include "miter.h"
#include "rnd_checker.h"

namespace eda::gate::debugger {

class RNDChecker : public BaseChecker, public util::Singleton<RNDChecker> {
friend class util::Singleton<RNDChecker>;


public:
  bool areEqual(GNet &lhs,
                GNet &rhs,
                Checker::GateIdMap &gmap) override;
};
} // namespace eda::gate::debugger
