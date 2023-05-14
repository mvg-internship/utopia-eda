//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/encoder.h"

#include <cassert>

namespace eda::gate::debugger {

void Encoder::encode(const GNet &net, uint16_t version) {
  for (const auto *gate: net.gates()) {
    encode(*gate, version);
  }
}

void Encoder::encode(const Gate &gate, uint16_t version) {
  using GateSymbol = eda::gate::model::GateSymbol;

  switch (gate.func()) {
  case GateSymbol::IN:
    break;
  case GateSymbol::OUT:
    encodeBuf(gate, true, version);
    break;
  case GateSymbol::ONE:
    encodeFix(gate, true, version);
    break;
  case GateSymbol::ZERO:
    encodeFix(gate, false, version);
    break;
  case GateSymbol::NOP:
    encodeBuf(gate, true, version);
    break;
  case GateSymbol::NOT:
    encodeBuf(gate, false, version);
    break;
  case GateSymbol::AND:
    encodeAnd(gate, true, version);
    break;
   case GateSymbol::NAND:
    encodeAnd(gate, false, version);
    break;
  case GateSymbol::OR:
    encodeOr(gate, true, version);
    break;
  case GateSymbol::NOR:
    encodeOr(gate, false, version);
    break;
  case GateSymbol::XOR:
    encodeXor(gate, true, version);
    break;
  case GateSymbol::XNOR:
    encodeXor(gate, false, version);
    break;
  case GateSymbol::MAJ:
    encodeMaj(gate, true, version);
    break;
  case GateSymbol::LATCH:
    encodeLatch(gate, version);
    break;
  case GateSymbol::DFF:
    encodeDff(gate, version);
    break;
  case GateSymbol::DFFrs:
    encodeDffRs(gate, version);
    break;
  default:
    assert(false && "Unsupported gate");
    break;
  }
}

void Encoder::encodeFix(const Gate &gate, bool sign, uint16_t version) {
  const auto y = _context.var(gate, version, Context::SET);

  encodeFix(y, sign);
}

void Encoder::encodeBuf(const Gate &gate, bool sign, uint16_t version) {
  const auto x = _context.var(gate.input(0), version, Context::GET);
  const auto y = _context.var(gate, version, Context::SET);

  encodeBuf(y, x, sign);
}

void Encoder::encodeAnd(const Gate &gate, bool sign, uint16_t version) {
  const auto y = _context.var(gate, version, Context::SET);
  Context::Clause clause;
  
  clause.push(Context::lit(y, sign));
  for (const auto &input : gate.inputs()) {
    const auto x = _context.var(input, version, Context::GET);

    clause.push(Context::lit(x, false));
    encode(Context::lit(y, !sign), Context::lit(x, true));
  }

  encode(clause);
}

void Encoder::encodeOr(const Gate &gate, bool sign, uint16_t version) {
  const auto y = _context.var(gate, version, Context::SET);
  Context::Clause clause;
 
  clause.push(Context::lit(y, !sign));
  for (const auto &input : gate.inputs()) {
    const auto x = _context.var(input, version, Context::GET);

    clause.push(Context::lit(x, true));
    encode(Context::lit(y, sign), Context::lit(x, false));
  }

  encode(clause);
}

void Encoder::encodeXor(const Gate &gate, bool sign, uint16_t version) {
  if (gate.arity() == 1) {
    return encodeBuf(gate, sign, version);
  }

  auto y = _context.var(gate, version, Context::SET);
  for (std::size_t i = 0; i < gate.arity() - 1; i++) {
    const auto x1 = _context.var(gate.input(i), version, Context::GET);
    const auto x2 = (i == gate.arity() - 2)
      ? _context.var(gate.input(i + 1), version, Context::GET)
      : _context.newVar();

    encodeXor(y, x1, x2, sign, true, true);
    y = x2;
  }
}

void Encoder::encodeMaj(const Gate &gate, bool sign, uint16_t version) {
  assert(gate.arity() == 3);
  auto y = _context.var(gate, version, Context::SET);
  auto x1 = _context.var(gate.input(0), version, Context::GET);
  auto x2 = _context.var(gate.input(1), version, Context::GET);
  auto x3 = _context.var(gate.input(2), version, Context::GET);
  encodeMaj(y, x1, x2, x3, sign, true, true, true);
}

void Encoder::encodeLatch(const Gate &gate, uint16_t version) {
  if (version == 0) { return; }

  // D latch (Q; D, ENA):
  // Q(t) = ENA(level1) ? D : Q(t-1).
  assert(gate.arity() == 2);

  const auto Qt  = _context.var(gate, version, Context::SET);
  const auto Qp  = _context.var(gate, version, Context::GET);
  const auto D   = _context.var(gate.input(0), version, Context::GET);
  const auto ENA = _context.var(gate.input(1), version, Context::GET);

  encodeMux(Qt, ENA, D, Qp, true);
}

void Encoder::encodeDff(const Gate &gate, uint16_t version) {
  if (version == 0) { return; }

  // D flip-flop (Q; D, CLK):
  // Q(t) = CLK(posedge) ? D : Q(t-1).
  assert(gate.arity() == 2);

  const auto Qt = _context.var(gate, version, Context::SET);
  const auto D  = _context.var(gate.input(0), version, Context::GET);

  // ASSUME: Design is synchronous: Q(t) = D.
  encodeBuf(Qt, D, true);
}

void Encoder::encodeDffRs(const Gate &gate, uint16_t version) {
  if (version == 0) { return; }

  // D flip-flop w/ (asynchronous) reset and set (Q; D, CLK, RST, SET):
  // Q(t) = RST(level1) ? 0 : (SET(level1) ? 1 : (CLK(posedge) ? D : Q(t-1))).
  assert(gate.arity() == 4);

  const auto Qt  = _context.var(gate, version, Context::SET);
  const auto D   = _context.var(gate.input(0), version, Context::GET);
  const auto RST = _context.var(gate.input(2), version, Context::GET);
  const auto SET = _context.var(gate.input(3), version, Context::GET);
  const auto TMP = _context.newVar();

  // ASSUME: Design is synchronous: Q(t) = ~RST & (SET | D).
  encodeAnd(Qt, RST, TMP, true, false, true);
  encodeOr(TMP, SET, D, true, true, true);
}

void Encoder::encodeFix(uint64_t y, bool s) {
  encode(Context::lit(y, s));
}

void Encoder::encodeBuf(uint64_t y, uint64_t x, bool s) {
  encode(Context::lit(y, !s), Context::lit(x, true));
  encode(Context::lit(y,  s), Context::lit(x, false));
}

void Encoder::encodeAnd(uint64_t y, uint64_t x1, uint64_t x2, bool s, bool s1, bool s2) {
  encode(Context::lit(y,  s), Context::lit(x1, !s1), Context::lit(x2, !s2));
  encode(Context::lit(y, !s), Context::lit(x1,  s1));
  encode(Context::lit(y, !s), Context::lit(x2,  s2));
}

void Encoder::encodeOr(uint64_t y, uint64_t x1, uint64_t x2, bool s, bool s1, bool s2) {
  encode(Context::lit(y, !s), Context::lit(x1,  s1), Context::lit(x2, s2));
  encode(Context::lit(y,  s), Context::lit(x1, !s1));
  encode(Context::lit(y,  s), Context::lit(x2, !s2));
}

void Encoder::encodeXor(uint64_t y, uint64_t x1, uint64_t x2, bool s, bool s1, bool s2) {
  encode(Context::lit(y, !s), Context::lit(x1, !s1), Context::lit(x2, !s2));
  encode(Context::lit(y, !s), Context::lit(x1,  s1), Context::lit(x2,  s2));
  encode(Context::lit(y,  s), Context::lit(x1, !s1), Context::lit(x2,  s2));
  encode(Context::lit(y,  s), Context::lit(x1,  s1), Context::lit(x2, !s2));
}

void Encoder::encodeMaj(uint64_t y, uint64_t x1, uint64_t x2, uint64_t x3,
                        bool s, bool s1, bool s2, bool s3) {
  const auto t1 = _context.newVar();
  const auto t2 = _context.newVar();
  const auto t3 = _context.newVar();

  // t1 = (x1 & x2), t2 = (x1 & x3), t3 = (x2 & x3).
  encodeAnd(t1, x1, x2, true, s1, s2);
  encodeAnd(t2, x1, x3, true, s1, s3);
  encodeAnd(t3, x2, x3, true, s2, s3);

  // y = maj(x1, x2, x3) = (t1 | t2 | t3).
  Context::Clause clause;
  clause.push(Context::lit(y, !s));
  clause.push(Context::lit(t1, true));
  clause.push(Context::lit(t2, true));
  clause.push(Context::lit(t3, true));
  encode(clause);

  encode(Context::lit(y, s), Context::lit(t1, false));
  encode(Context::lit(y, s), Context::lit(t2, false));
  encode(Context::lit(y, s), Context::lit(t3, false));
}

void Encoder::encodeMux(uint64_t y, uint64_t c, uint64_t x1, uint64_t x2, bool s) {
  const auto t1 = _context.newVar();
  const auto t2 = _context.newVar();

  // y = (t1 | t2), where t1 = (c & x1) and t2 = (~c & x2).
  encodeOr(y, t1, t2, s, true, true);
  encodeAnd(t1, c, x1, true, true, true);
  encodeAnd(t2, c, x2, true, false, true);
}

} // namespace eda::gate::debugger
