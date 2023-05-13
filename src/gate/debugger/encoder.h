//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/debugger/context.h"
#include "gate/model/gnet.h"

#include "minisat/core/Solver.h"

#include <string>
#include <vector>

namespace eda::gate::debugger {

/**
 * \brief Implements a Tseitin encoder of a gate-level netlist.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class Encoder final {
  using Gate = eda::gate::model::Gate;
  using GNet = eda::gate::model::GNet;

public:
  void encode(const GNet &net, uint16_t version);
  void encode(const Gate &gate, uint16_t version);

  // Combinational gates.
  void encodeFix(const Gate &gate, bool sign, uint16_t version);
  void encodeBuf(const Gate &gate, bool sign, uint16_t version);
  void encodeAnd(const Gate &gate, bool sign, uint16_t version);
  void encodeOr (const Gate &gate, bool sign, uint16_t version);
  void encodeXor(const Gate &gate, bool sign, uint16_t version);
  void encodeMaj(const Gate &gate, bool sign, uint16_t version);

  // Latches and flip-flops.
  void encodeLatch(const Gate &gate, uint16_t version);
  void encodeDff  (const Gate &gate, uint16_t version);
  void encodeDffRs(const Gate &gate, uint16_t version);

  /// Encodes the equality y == s.
  void encodeFix(uint64_t y, bool s);
  /// Encodes the equality y^s == x.
  void encodeBuf(uint64_t y, uint64_t x, bool s);
  /// Encodes the equality y^s == x1^s1 | x2^s2.
  void encodeAnd(uint64_t y, uint64_t x1, uint64_t x2, bool s, bool s1, bool s2);
  /// Encodes the equality y^s == x1^s1 | x2^s2.
  void encodeOr (uint64_t y, uint64_t x1, uint64_t x2, bool s, bool s1, bool s2);
  /// Encodes the equality y^s == x1^s1 ^ x2^s2.
  void encodeXor(uint64_t y, uint64_t x1, uint64_t x2, bool s, bool s1, bool s2);
  /// Encodes the equality y^s == maj(x1^s1, x2^s2, x3^s3).
  void encodeMaj(uint64_t y, uint64_t x1, uint64_t x2, uint64_t x3,
                 bool s, bool s1, bool s2, bool s3);
  /// Encodes the equality y^s == c ? x1 : x2.
  void encodeMux(uint64_t y, uint64_t c, uint64_t x1, uint64_t x2, bool s);

  void encode(Context::Lit lit) {
    _context.solver().addClause(lit);
  }

  void encode(Context::Lit lit1, Context::Lit lit2) {
    _context.solver().addClause(lit1, lit2);
  }

  void encode(Context::Lit lit1, Context::Lit lit2, Context::Lit lit3) {
    _context.solver().addClause(lit1, lit2, lit3);
  }

  void encode(const Context::Clause &clause) {
    _context.solver().addClause(clause);
  }

  Context& context() {
    return _context;
  }

  void setConnectTo(const Context::GateConnect *connectTo) {
    _context.setConnectTo(connectTo);
  }

  uint64_t var(Gate::Id gateId, uint16_t version) {
    return _context.var(gateId, version);
  }

  uint64_t newVar() {
    return _context.newVar();
  }

  bool solve() {
    return _context.solver().solve();
  }

private:
  Context _context;
};

} // namespace eda::gate::debugger
