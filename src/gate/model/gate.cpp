//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gate.h"

#include <iostream>

namespace eda::gate::model {

static std::ostream &operator <<(std::ostream &out, const Gate::SignalList &signals) {
  bool separator = false;
  for (const Gate::Signal &signal: signals) {
    out << (separator ? ", " : "") << signal.event() << "(" << signal.node() << ")";
    separator = true;
  }
  return out;
}

std::ostream &operator <<(std::ostream &out, const Gate &gate) {
  out << "G{" << gate.id() << " <= " << gate.func() << "(" << gate.inputs() << ")}";
  return out << "[fo=" << gate.fanout() << "]";
}

} // namespace eda::gate::model
