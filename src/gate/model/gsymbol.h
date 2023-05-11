//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <iostream>
#include <vector>

namespace eda::gate::model {

/**
 * \brief Defines names of supported logical gates and flip-flops/latches.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class GateSymbol final {
public:
  enum Value : uint16_t {
    //--------------------------------------------------------------------------
    // Input/output pseudo-gates
    //--------------------------------------------------------------------------

    /// Primary input.
    IN,
    /// Primary output.
    OUT,

    //--------------------------------------------------------------------------
    // Logic gates
    //--------------------------------------------------------------------------

    /// Constant 0: OUT = 0.
    ZERO,
    /// Constant 1: OUT = 1.
    ONE,
    /// Identity: OUT = X.
    NOP,
    /// Negation: OUT = ~X.
    NOT,
    /// Conjunction: OUT = X & Y (& ...).
    AND,
    /// Disjunction: OUT = X | Y (| ...).
    OR,
    /// Exclusive OR: OUT = X + Y (+ ...) (mod 2).
    XOR,
    /// Sheffer's stroke: OUT = ~(X & Y (& ...)).
    NAND,
    /// Peirce's arrow: OUT = ~(X | Y (| ...)).
    NOR,
    /// Exclusive NOR: OUT = ~(X + Y (+ ...) (mod 2)).
    XNOR,
    /// Majority function: OUT = Majority(X, Y, ...).
    MAJ,

    //--------------------------------------------------------------------------
    // Flip-flops and latches
    //--------------------------------------------------------------------------

    /// D latch (Q, D, ENA):
    /// Q(t) = ENA(level1) ? D : Q(t-1).
    LATCH,
    /// D flip-flop (Q, D, CLK):
    /// Q(t) = CLK(posedge) ? D : Q(t-1).
    DFF,
    /// D flip-flop w/ (asynchronous) reset and set (Q, D, CLK, RST, SET):
    /// Q(t) = RST(level1) ? 0 : (SET(level1) ? 1 : (CLK(posedge) ? D : Q(t-1))).
    DFFrs,

    /// Number of pre-defined gate symbols.
    XXX
  };

  /// Creates an uninterpreted custom gate symbol.
  static GateSymbol create(const std::string &name);

private:
  constexpr static size_t N_GATE_SYMBOLS = 16536;

  struct GateDescriptor {
    std::string name;
    bool isConstant;
    bool isIdentity;
    bool isCommutative;
    bool isAssociative;
    bool isDecomposable;
    Value modifier;
    Value function;
  };

  constexpr const GateDescriptor &desc() const {
    return _desc[_value];
  }

public:
  GateSymbol() = default;
  constexpr GateSymbol(Value value): _value(value) {}

  explicit operator bool() const = delete;
  constexpr operator Value() const { return _value; }

  constexpr const std::string &name() const { return desc().name; }

  constexpr bool isConstant()     const { return desc().isConstant; }
  constexpr bool isIdentity()     const { return desc().isIdentity; }
  constexpr bool isCommutative()  const { return desc().isCommutative; }
  constexpr bool isAssociative()  const { return desc().isAssociative; }
  constexpr bool isDecomposable() const { return desc().isDecomposable; }

  constexpr GateSymbol modifier() const { return desc().modifier; }
  constexpr GateSymbol function() const { return desc().function; }

private:
  Value _value;

  static GateDescriptor _desc[N_GATE_SYMBOLS];
  static uint16_t _next;
};

std::ostream& operator <<(std::ostream &out, GateSymbol gate);

} // namespace eda::gate::model
