//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

#include "minisat/core/Solver.h"

#include <cstdint>
#include <unordered_map>

using namespace eda::gate::model;

namespace eda::gate::debugger {

/**
 * \brief Logic formula representing a gate-level net.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class Context final {
public:
  // MiniSAT-related types.
  using Var = Minisat::Var;
  using Lit = Minisat::Lit;
  using Clause = Minisat::vec<Lit>;
  using Solver = Minisat::Solver;

  // Gate reconnection map.
  using GateConnect = std::unordered_map<Gate::Id, Gate::Id>;

  // Signal access mode.
  enum Mode { GET, SET };

  /// Creates a literal.
  static Lit lit(uint64_t var, bool sign) {
    return Minisat::mkLit(static_cast<Var>(var), sign);
  }

  /// Returns a variable id.
  uint64_t var(Gate::Id gateId, uint16_t version) {
    return reserve(var(_connectTo, gateId, version));
  }

  /// Returns a variable id.
  uint64_t var(const Gate &gate, uint16_t version, Mode mode) {
    return (mode == GET && gate.isTrigger() && version > 0)
      ? var(gate.id(), version - 1)
      : var(gate.id(), version);
  }

  /// Returns a variable id.
  uint64_t var(const Gate::Signal &signal, uint16_t version, Mode mode) {
    return var(*Gate::get(signal.node()), version, mode);
  }

  /// Returns a new variable id.
  uint64_t newVar() {
    // See the variable id format.
    static uint64_t var = 0;
    return reserve(((var++) << 1) | 1);
  }

  /// Returns the variable value.
  bool value(uint64_t var) {
    return _solver.modelValue(static_cast<Var>(var)) == Minisat::l_True;
  }

  /// Dumps the current formula to the file.
  void dump(const std::string &file) {
    _solver.toDimacs(file.c_str());
  }

  /// Sets the gate reconnection map.
  void setConnectTo(const GateConnect *connectTo) {
    _connectTo = connectTo;
  }

  /// Returns the SAT solver instance.
  Solver& solver() {
    return _solver;
  }

private:
  /**
   * Returns a variable id, which is an integer of the following format:
   *
   * |0..0|Version|GateId|New|
   *    2     8      20   (1)  32 bits [as is]
   *  (16)  (16)    (31)  (1)  64 bits [to be]
   *
   * The version is used for symbolic execution and can borrow bits for id.
   * The current limitations on the field widths are caused by MiniSAT.
   */
  static uint64_t var(const GateConnect *connectTo,
                      Gate::Id gateId,
                      uint16_t version) {
    // FIXME: Such encoding is not suitable for MiniSAT w/ IntMap implemented as a vector.
    return ((uint64_t)version << 21) |
           ((uint64_t)connectedTo(connectTo, gateId) << 1);
  }

  /// Returns the gate id the given one is connected to.
  static Gate::Id connectedTo(const GateConnect *connectTo, Gate::Id gateId) {
    if (connectTo) {
      auto i = connectTo->find(gateId);
      if (i != connectTo->end())
        return i->second;
    }

    return gateId;
  }

  /// Allocates the variable in the SAT solver.
  uint64_t reserve(uint64_t var) {
    while (var >= (uint64_t)_solver.nVars()) {
      _solver.newVar();
    }
    return var;
  }

  const GateConnect *_connectTo = nullptr;
  Solver _solver;
};

} // namespace eda::gate::debugger
