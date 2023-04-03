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
#include "rtl/model/fsymbol.h"

#include <cstddef>
#include <map>
#include <memory>
#include <utility>
#include <vector>

using GNet = eda::gate::model::GNet;
using GateId = GNet::GateId;
using GateIdList = eda::rtl::library::FLibrary::GateIdList;
using FuncSymbol = eda::rtl::model::FuncSymbol;

namespace eda::rtl::library {

// Complete GateIdList with zeros up to the passed size:
// 111 -> 000111
void fillingWithZeros(const size_t size,
                      GateIdList &in,
                      GNet &net);

// Make inputs equal to each other,
// but no longer than outSize
inline void makeInputsEqual(const size_t outSize,
                            GateIdList &x,
                            GateIdList &y,
                            GNet &net);

// Form GateIdList of outputs for the operation
// applied to pairs of input identifiers
GateIdList formGateIdList(const size_t size,
                          GateSymbol func,
                          const GateIdList &x,
                          const GateIdList &y,
                          GNet &net);

// Divide one GateIdList into two GateIdLists:
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

/**
 * \brief Library for arithmetic operations.
 * \author <a href="mailto:alexei.yagzhov@yandex.ru">Alexey Yagzhov</a>
 */
class ArithmeticLibrary final : public FLibrary {
public:
  using GateIdKey = std::pair<size_t, int>;
  using GateIdTree = std::map<GateIdKey, GateId>;

  static FLibrary &get() {
    static auto instance = std::unique_ptr<FLibrary>(
                                 new ArithmeticLibrary(FLibraryDefault::get()));
    return *instance;
  }

  bool supports(FuncSymbol func) const override;

  Out synth(size_t outSize, 
            const Value &value, 
            GNet &net) override;

  Out synth(size_t outSize, 
            const Out &out, 
            GNet &net) override;

  Out synth(size_t outSize, 
            FuncSymbol func, 
            const In &in, 
            GNet &net) override;

  Out synth(const Out &out, 
            const In &in, 
            const SignalList &control, 
            GNet &net) override;

  Out alloc(size_t outSize, 
            GNet &net) override;

private:
  ArithmeticLibrary(FLibrary &library) : supportLibrary(library) {}
  ~ArithmeticLibrary() override {}

  static Out synthAdd(size_t outSize, 
                      const In &in, 
                      GNet &net);

  static Out synthSub(size_t outSize, 
                      const In &in, 
                      GNet &net);

  static Out synthLadnerFisherAdder(const size_t outSize, 
                                    const In &in, 
                                    const bool plusOne, 
                                    GNet &net);

  static Out synthKaratsubaMultiplier(const size_t outSize,
                                      const In &in,
                                      const size_t depth,
                                      GNet &net);

  static Out synthColumnMultiplier(const size_t outSize,
                                   const In &in,
                                   GNet &net);

  static Out synthMultiplierByOneDigit(const size_t outSize,
                                       const GateIdList &x,
                                       const GateId &y,
                                       GNet &net);

  FLibrary &supportLibrary;
};

} // namespace eda::rtl::library
