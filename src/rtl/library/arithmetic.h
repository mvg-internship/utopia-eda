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

using namespace eda::gate::model;
using namespace eda::rtl::model;

namespace eda::rtl::library {

class ArithmeticLibrary final : public FLibrary {
public:
  using Key = std::pair<size_t, int>;
  using SignalTree = std::map<Key, Signal>;

  static FLibrary &get() {
    static auto instance = std::unique_ptr<FLibrary>(new ArithmeticLibrary(FLibraryDefault::get()));
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

  static Out synthAdder(size_t outSize, 
                        const In &in, 
                        bool plusOne, 
                        GNet &net);

  // Complements in with zeros to the transmitted size, if necessary 
  static inline void filling(size_t size, 
                             GateIdList &in, 
                             GNet &net);

  // Forms signals for input identifiers 
  static inline SignalList formInputSignals(size_t size, 
                                            GateIdList in);
  
  static inline SignalList formCarrySignals(size_t size, 
                                            GateSymbol func, 
                                            SignalList &xWire, 
                                            SignalList &yWire, 
                                            GNet &net);

  FLibrary &supportLibrary;
};

} // namespace eda::rtl::library

