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
using GateIdList = eda::rtl::library::FLibrary::GateIdList;
using FuncSymbol = eda::rtl::model::FuncSymbol;

namespace eda::rtl::library {

/**
 * \brief Library for arithmetic operations.
 * \author <a href="mailto:aayagzhov@yandex.ru">Alexey Yagzhov</a>
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

  bool supports(const FuncSymbol func) const override;

  Out synth(const size_t outSize,
            const Value &value,
            GNet &net) override;

  Out synth(const size_t outSize,
            const Out &out,
            GNet &net) override;

  Out synth(const size_t outSize,
            const FuncSymbol func,
            const In &in, 
            GNet &net) override;

  Out synth(const Out &out,
            const In &in,
            const SignalList &control,
            GNet &net) override;

  Out alloc(const size_t outSize,
            GNet &net) override;

private:
  ArithmeticLibrary(FLibrary &library) : supportLibrary(library) {}
  ~ArithmeticLibrary() override {}

  static Out synthAdd(const size_t outSize,
                      const In &in,
                      GNet &net);

  static Out synthSub(const size_t outSize,
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
