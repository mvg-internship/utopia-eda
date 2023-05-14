//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "base/model/link.h"
#include "base/model/node.h"
#include "base/model/signal.h"
#include "gate/model/gsymbol.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace eda::rtl::compiler {
  class Compiler;
} // namespace eda::rtl::compiler

namespace eda::gate::model {

class GNet;

using GateBase = eda::base::model::Node<GateSymbol, true>;

/**
 * \brief Represents a logic gate or a flip-flop/latch.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class Gate final : public GateBase {
  // To create gates when synthesizing netlists.
  friend class GNet;
  friend class eda::rtl::compiler::Compiler;

public:
  using List = std::vector<Gate*>;
 
  /// Returns the gate w/ the given id.
  static Gate *get(Id id) {
    return static_cast<Gate*>(GateBase::get(id));
  }

  /// Returns the gate w/ the given function and inputs.
  static Gate *get(uint32_t netId, GateSymbol func, const SignalList &inputs) {
    return static_cast<Gate*>(GateBase::get(netId, func, inputs));
  }

  /// Saves the gate in the hash table.
  static void add(uint32_t netId, Gate *gate) {
    GateBase::add(netId, gate);
  }

  bool isSource() const {
    return _func == GateSymbol::IN;
  }

  bool isTarget() const {
    return _func == GateSymbol::OUT;
  }

  bool isValue() const {
    return _func == GateSymbol::ONE || _func == GateSymbol::ZERO;
  }

  bool isTrigger() const {
    for (const auto &input: _inputs) {
      if (!input.isAlways())
        return true;
    }
    return false;
  }

  bool isComb() const {
    return !isSource() && !isTarget() && !isTrigger();
  }

private:
  /// Creates a gate w/ the given operation and the inputs.
  Gate(GateSymbol func, const SignalList &inputs): GateBase(func, inputs) {}

  /// Creates a source gate.
  Gate(): Gate(GateSymbol::IN, {}) {}

  /// Invariant.
  bool invariant() const {
    return // Source <=> no inputs.
           ((isSource() || isValue()) == (arity() == 0))
           // Target ==> no outputs.
        && (!isTarget() || (fanout() == 0));
  }
};

//===----------------------------------------------------------------------===//
// Signal Utilities
//===----------------------------------------------------------------------===//

inline bool isValue(const Gate::Signal &arg) {
  const auto *gate = Gate::get(arg.node());
  return gate->isValue();
}

inline bool isZero(const Gate::Signal &arg) {
  const auto *gate = Gate::get(arg.node());
  return gate->isValue() && gate->func() == GateSymbol::ZERO;
}

inline bool isOne(const Gate::Signal &arg) {
  const auto *gate = Gate::get(arg.node());
  return gate->isValue() && gate->func() == GateSymbol::ONE;
}

inline bool areIdentical(const Gate::Signal &lhs, const Gate::Signal &rhs) {
  return lhs.node() == rhs.node();
}

inline bool areContrary(const Gate::Signal &lhs, const Gate::Signal &rhs) {
  const auto *lhsGate = Gate::get(lhs.node());
  const auto *rhsGate = Gate::get(rhs.node());

  if (lhsGate->func() == GateSymbol::NOT) {
    return areIdentical(lhsGate->input(0), rhs); 
  }
  if (rhsGate->func() == GateSymbol::NOT) {
    return areIdentical(lhs, rhsGate->input(0));
  }

  return false;
}

//===----------------------------------------------------------------------===//
// Output
//===----------------------------------------------------------------------===//

std::ostream &operator <<(std::ostream &out, const Gate &gate);

} // namespace eda::gate::model
