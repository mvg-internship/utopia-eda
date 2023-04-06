//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "gate/simulator/simulator.h"
#include "rtl/library/flibrary.h"

#include <cassert>
#include <cmath>

using GNet = eda::gate::model::GNet;

namespace eda::gate::debugger {

// generator return value
// EQUAL returns if there exhaustive check and nets are equal
// UNKNOWN returns if there NO exhaustive check and nets are equal
enum Result {
  ERROR = -2,
  UNKNOWN = -1,
  EQUAL = 0,
  NOTEQUAL = 1,
};

Result Generator(GNet &miter, const unsigned int tries, const bool exhaustive);
} // namespace eda::gate::debugger
