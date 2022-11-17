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

using namespace eda::gate::model;

namespace eda::gate::debugger {

/**
 * \brief Implements a symbolic executor of gate-level nets.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class SymbolicExecutor final {
public:
  SymbolicExecutor(): _cycle(1 /* should be positive */) {}

  void exec(const GNet &net);
  void exec(const GNet &net, unsigned cycles);

  void tick() { _cycle++; }

  unsigned cycle() const { return _cycle; }
  Context& context() { return _encoder.context(); }

private:
  unsigned _cycle;
  Encoder _encoder;
};

} // namespace eda::gate::debugger
