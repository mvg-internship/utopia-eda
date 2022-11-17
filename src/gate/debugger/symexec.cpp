//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/encoder.h"
#include "gate/debugger/symexec.h"

#include <cassert>

namespace eda::gate::debugger {

void SymbolicExecutor::exec(const GNet &net) {
  _encoder.encode(net, _cycle);
}

void SymbolicExecutor::exec(const GNet &net, unsigned cycles) {
  for (unsigned i = 0; i < cycles; i++) {
    exec(net);
    tick();
  }
}

} // namespace eda::gate::debugger
