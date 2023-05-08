//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/utils.h"
#include "gate/premapper/aigmapper.h"

#include <cassert>
#include <unordered_set>

namespace eda::gate::premapper {

using Gate = eda::gate::model::Gate;
using GNet = eda::gate::model::GNet;

Gate::Id AigMapper::mapGate(const Gate &oldGate,
                            const GateIdMap &oldToNewGates,
                            GNet &newNet) const {
  using GateSymbol = eda::gate::model::GateSymbol;

  if (oldGate.isSource() || oldGate.isTrigger()) {
    // Clone sources and triggers gates w/o changes.
    return PreMapper::mapGate(oldGate, oldToNewGates, newNet);
  }

  size_t n0, n1;
  auto newInputs = model::getNewInputs(oldGate, oldToNewGates, n0, n1);

  switch (oldGate.func()) {
  case GateSymbol::IN   : return mapIn (                          newNet);
  case GateSymbol::OUT  : return mapOut(newInputs, n0, n1,        newNet);
  case GateSymbol::ZERO : return mapVal(                   false, newNet);
  case GateSymbol::ONE  : return mapVal(                   true,  newNet);
  case GateSymbol::NOP  : return mapNop(newInputs, n0, n1, true,  newNet);
  case GateSymbol::NOT  : return mapNop(newInputs, n0, n1, false, newNet);
  case GateSymbol::AND  : return mapAnd(newInputs, n0, n1, true,  newNet);
  case GateSymbol::OR   : return mapOr (newInputs, n0, n1, true,  newNet);
  case GateSymbol::XOR  : return mapXor(newInputs, n0, n1, true,  newNet);
  case GateSymbol::NAND : return mapAnd(newInputs, n0, n1, false, newNet);
  case GateSymbol::NOR  : return mapOr (newInputs, n0, n1, false, newNet);
  case GateSymbol::XNOR : return mapXor(newInputs, n0, n1, false, newNet);
  default: assert(false && "Unknown gate");
  }

  return Gate::INVALID;
}

//===----------------------------------------------------------------------===//
// IN/OUT
//===----------------------------------------------------------------------===//

Gate::Id AigMapper::mapIn(GNet &newNet) const {
  return newNet.addIn();
}

Gate::Id AigMapper::mapOut(const Gate::SignalList &newInputs,
                           const size_t n0,
                           const size_t n1,
                           GNet &newNet) const {
  assert((newInputs.size() + n0 + n1) == 1 && "Only single input is allowed");

  // Constant output.
  if (n0 > 0 || n1 > 0) {
    auto valId = mapVal(n1 > 0, newNet);
    return newNet.addOut(valId);
  }

  return newNet.addOut(newInputs);
}

//===----------------------------------------------------------------------===//
// ONE/ZERO
//===----------------------------------------------------------------------===//

Gate::Id AigMapper::mapVal(const bool value, GNet &newNet) const {
  return value ? newNet.addOne() : newNet.addZero();
}

//===----------------------------------------------------------------------===//
// NOP/NOT
//===----------------------------------------------------------------------===//

Gate::Id AigMapper::mapNop(const Gate::SignalList &newInputs,
                           const bool sign, GNet &newNet) const {
  // NOP(x) = x.
  const auto inputId = newInputs.at(0).node();
  if (sign) {
    return inputId;
  }

  // NOT(NOT(x)) = x.
  const auto *inputGate = Gate::get(inputId);
  if (inputGate->func() == eda::gate::model::GateSymbol::NOT) {
    return inputGate->input(0).node();
  }

  // NOT(x).
  return newNet.addNot(newInputs);
}

Gate::Id AigMapper::mapNop(const Gate::SignalList &newInputs,
                           const size_t n0,
                           const size_t n1,
                           const bool sign,
                           GNet &newNet) const {
  assert((newInputs.size() + n0 + n1) == 1 && "Only single input is allowed");

  if (n0 > 0 || n1 > 0) {
    return mapVal((n0 > 0) ^ sign, newNet);
  }

  return mapNop(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// AND/NAND
//===----------------------------------------------------------------------===//

Gate::Id AigMapper::mapAnd(const Gate::SignalList &newInputs,
                           const bool sign, GNet &newNet) const {
  Gate::SignalList inputs(newInputs.begin(), newInputs.end());
  inputs.reserve(2 * newInputs.size() - 1);

  size_t l = 0;
  size_t r = 1;
  while (r < inputs.size()) {
    const auto x = inputs[l];
    const auto y = inputs[r];

    Gate::Id gateId;
    if (model::areIdentical(x, y)) {
      // AND(x,x) = x.
      gateId = mapNop({x}, sign, newNet);
    } else if (model::areContrary(x, y)) {
      // AND(x,NOT(x)) = 0.
      gateId = mapVal(!sign, newNet);
    } else {
      // AND(x,y).
      gateId = newNet.addAnd(x, y);
    }

    inputs.push_back(Gate::Signal::always(gateId));

    l += 2;
    r += 2;
  }

  return mapNop({inputs[l]}, sign, newNet);
}

Gate::Id AigMapper::mapAnd(const Gate::SignalList &newInputs,
                           const size_t n0,
                           const size_t n1,
                           const bool sign,
                           GNet &newNet) const {
  if (n0 > 0) {
    return mapVal(!sign, newNet);
  }

  return mapAnd(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// OR/NOR
//===----------------------------------------------------------------------===//

Gate::Id AigMapper::mapOr(const Gate::SignalList &newInputs,
                          bool sign, GNet &newNet) const {
  // OR(x[1],...,x[n]) = NOT(AND(NOT(x[1]),...,NOT(x[n]))).
  Gate::SignalList negInputs(newInputs.size());
  for (size_t i = 0; i < newInputs.size(); i++) {
    const auto negInputId = mapNop({newInputs[i]}, false, newNet);
    negInputs[i] = Gate::Signal::always(negInputId);
  }

  return mapAnd(negInputs, !sign, newNet);
}

Gate::Id AigMapper::mapOr(const Gate::SignalList &newInputs,
                          size_t n0,
                          size_t n1,
                          bool sign,
                          GNet &newNet) const {
  if (n1 > 0) {
    return mapVal(sign, newNet);
  }

  return mapOr(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// XOR/XNOR
//===----------------------------------------------------------------------===//

Gate::Id AigMapper::mapXor(const Gate::SignalList &newInputs,
                           bool sign, GNet &newNet) const {
  Gate::SignalList inputs(newInputs.begin(), newInputs.end());
  inputs.reserve(2 * newInputs.size() - 1);

  size_t l = 0;
  size_t r = 1;
  while (r < inputs.size()) {
    // XOR (x,y)=AND(NAND(x,y),NAND(NOT(x),NOT(y))): 7 AND and NOT gates.
    // XNOR(x,y)=AND(NAND(x,NOT(y)),NAND(NOT(x),y)): 7 AND and NOT gates.
    const auto x1 = mapNop({inputs[l]},  true, newNet);
    const auto y1 = mapNop({inputs[r]},  sign, newNet);
    const auto x2 = mapNop({inputs[l]}, false, newNet);
    const auto y2 = mapNop({inputs[r]}, !sign, newNet);

    const auto z1 = mapAnd({Gate::Signal::always(x1), Gate::Signal::always(y1)},
                           false, newNet);
    const auto z2 = mapAnd({Gate::Signal::always(x2), Gate::Signal::always(y2)},
                           false, newNet);
    const auto id = mapAnd({Gate::Signal::always(z1), Gate::Signal::always(z2)},
                           true,  newNet);

    inputs.push_back(Gate::Signal::always(id));

    l += 2;
    r += 2;

    sign = true;
  }

  return inputs[l].node();
}

Gate::Id AigMapper::mapXor(const Gate::SignalList &newInputs,
                           size_t n0,
                           size_t n1,
                           bool sign,
                           GNet &newNet) const {
  bool newSign = sign ^ (n1 & 1);

  if (newInputs.empty()) {
    return mapVal(newSign, newNet);
  }
  
  return mapXor(newInputs, newSign, newNet);
}

} // namespace eda::gate::premapper
