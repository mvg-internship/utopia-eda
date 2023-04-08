//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "rtl/library/flibrary.h"

using FLibrary = eda::rtl::library::FLibrary;
using GateIdList = FLibrary::GateIdList;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;

namespace eda::rtl::library {

// Complete GateIdList with zeros up to the passed size:
// 111 -> 000111
void fillingWithZeros(const size_t size,
                      GateIdList &in,
                      GNet &net);

// Form GateIdList of outputs for the operation
// applied to pairs of input identifiers
GateIdList formGateIdList(const size_t size,
                          const GateSymbol func,
                          const GateIdList &x,
                          const GateIdList &y,
                          GNet &net);

// Make inputs equal to each other,
// but no longer than outSize
void makeInputsEqual(const size_t outsize,
                     GateIdList &x,
                     GateIdList &y,
                     GNet &net);

// Divide one gateidlist into two GateIdLists:
// 111000 -> 111 and 000 (for firstPartSize = 3)
void getPartsOfGateIdList(const GateIdList &x,
                          GateIdList &x1,
                          GateIdList &x0,
                          const size_t firstPartSize);

// Make left shift for GateIdList:
// 111 -> 111000 (for shift = 3)
GateIdList leftShiftForGateIdList(const GateIdList &x,
                                  const size_t shift,
                                  GNet &net);
} // namespace eda::rtl::library
