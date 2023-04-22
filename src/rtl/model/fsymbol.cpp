//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <iostream>

#include "rtl/model/fsymbol.h"

namespace eda::rtl::model {

std::ostream &operator <<(std::ostream &out, FuncSymbol func) {
  switch (func) {
  case FuncSymbol::NOP:
    return out << "";
  case FuncSymbol::NOT:
    return out << "~";
  case FuncSymbol::AND:
    return out << "&";
  case FuncSymbol::OR:
    return out << "|";
  case FuncSymbol::XOR:
    return out << "^";
  case FuncSymbol::ADD:
    return out << "+";
  case FuncSymbol::SUB:
    return out << "-";
  case FuncSymbol::MUL:
    return out << "*";
  case FuncSymbol::MUX:
    return out << "mux";
  }
  return out;
}

} // namespace eda::rtl::model
