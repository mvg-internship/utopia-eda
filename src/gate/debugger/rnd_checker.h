//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once
#include "base_checker.h"
#include "checker.h"
#include "gate/model/gnet.h"
#include "gate/simulator/simulator.h"
#include "miter.h"
#include "rtl/library/flibrary.h"

#include <cassert>
#include <cmath>

using GNet = eda::gate::model::GNet;

namespace eda::gate::debugger {

// generator return value
// EQUAL returns if there exhaustive check and nets are equal
// UNKNOWN returns if there NO exhaustive check and nets are equal
// NOTEQUAL returns if nets are not equal
// ERROR returns if invalid arguments were given
enum Result {
  ERROR = -2,
  UNKNOWN = -1,
  EQUAL = 0,
  NOTEQUAL = 1,
};

/**
 *  \brief Goes through values and checks miter output.
 *  @param miter Miter which will receive values.
 *  @param tries Number of random values checked, if the check is inexhaustive.
 *  @param exhaustive Sets the mode of the check.
 *  @return The result of the check.
 */
Result Generator(GNet &miter, const unsigned int tries, const bool exhaustive);

class RNDChecker : public BaseChecker, public util::Singleton<RNDChecker> {
friend class util::Singleton<RNDChecker>;

public:
  bool areEqual(GNet &lhs,
                GNet &rhs,
                Checker::GateIdMap &gmap) override;
  void setTries(int tries);
  void setExhaustive(bool exhaustive);
private:
  int tries = 0;
  bool exhaustive = true;
};

} // namespace eda::gate::debugger
