//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <iostream>

namespace eda::rtl::model {

/**
 * \brief Defines names of supported RTL-level functions.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
enum FuncSymbol {
  /// Identity: OUT = X.
  NOP,
  /// Negation: OUT = ~X.
  NOT,
  /// Conjunction: OUT = X & Y.
  AND,
  /// Disjunction: OUT = X | Y.
  OR,
  /// Exclusive OR: OUT = X ^ Y.
  XOR,
  /// Addition: OUT = X + Y.
  ADD,
  /// Subtraction: OUT = X - Y.
  SUB,
  /// Multiplication: OUT = X * Y.
  MUL,
  /// Multiplexor: OUT = MUX(C[1], ..., C[n]; X[1], ..., X[n]).
  MUX

  // TODO: Add more built-in functions.
  // TODO: Add user-defined (library) functions.
};

std::ostream &operator <<(std::ostream &out, FuncSymbol func);

} // namespace eda::rtl::model
