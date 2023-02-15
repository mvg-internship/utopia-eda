//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "rtl/model/fsymbol.h"

#include <cassert>
#include <memory>
#include <vector>

using namespace eda::gate::model;
using namespace eda::rtl::model;

namespace eda::gate::model {
  class GNet;
} // namespace eda::gate::model

namespace eda::rtl::library {

/**
 * \brief Interface for functional library.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>.
 */
struct FLibrary {
  using GateIdList = GNet::GateIdList;
  using Signal     = GNet::Signal;
  using SignalList = GNet::SignalList;
  using Value      = GNet::Value;
  using In         = GNet::In;
  using Out        = GNet::Out;

  /// Checks if the library supports the given function.
  virtual bool supports(FuncSymbol func) const = 0;

  /// Synthesizes the gate-level net for the given value.
  virtual Out synth(size_t outSize,
                    const Value &value,
                    GNet &net) = 0;

  /// Synthesizes the gate-level net for the given output.
  virtual Out synth(size_t outSize,
                    const Out &out,
                    GNet &net) = 0;

  /// Synthesizes the gate-level net for the given function.
  virtual Out synth(size_t outSize,
                    FuncSymbol func,
                    const In &in,
                    GNet &net) = 0;

  /// Synthesizes the gate-level net for the given register.
  virtual Out alloc(size_t outSize,
                    GNet &net) = 0;

  virtual Out synth(const Out &out, // allocated in advance
                    const In &in,
                    const SignalList &control,
                    GNet &net) = 0;

  virtual ~FLibrary() {} 
};

/**
 * \brief Functional library default implementation.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class FLibraryDefault final: public FLibrary {
public:
  static FLibrary& get() {
    static auto instance = std::unique_ptr<FLibrary>(new FLibraryDefault());
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

  Out alloc(size_t outSize,
            GNet &net) override;

  Out synth(const Out &out,
            const In &in,
            const SignalList &control,
            GNet &net) override;

private:
  FLibraryDefault() {}
  ~FLibraryDefault() override {}

  static Out synthAdd(size_t outSize, const In &in, GNet &net);
  static Out synthSub(size_t outSize, const In &in, GNet &net);
  static Out synthMux(size_t outSize, const In &in, GNet &net);

  static Out synthAdder(size_t size, const In &in, bool plusOne, GNet &net);

  /// Returns two-bit output: z and carryOut (if required).
  static Out synthAdder(Gate::Id x,
                        Gate::Id y,
                        Gate::Id carryIn,
                        bool needsCarryOut,
                        GNet &net);

  static Signal invertIfNegative(const Signal &event, GNet &net);

  static Out synthUnaryBitwiseOp(
      GateSymbol func, size_t outSize, const In &in, GNet &net);
  static Out synthBinaryBitwiseOp(
      GateSymbol func, size_t outSize, const In &in, GNet &net);
  static Out synthSimpleAdder(size_t outSize, const In &in, GNet &net);
};

} // namespace eda::rtl::library
