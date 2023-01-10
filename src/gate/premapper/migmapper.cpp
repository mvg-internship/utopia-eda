//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/migmapper.h"

#include <cassert>
#include <unordered_set>

namespace eda::gate::premapper {

Gate::SignalList getNewInputs(const Gate &oldGate,
                              const MigMapper::GateIdMap &oldToNewGates,
                              size_t &n0, size_t &n1) {
  const auto k = oldGate.arity();

  Gate::SignalList newInputs;
  newInputs.reserve(k);

  n0 = n1 = 0;
  for (auto input : oldGate.inputs()) {
    if (model::isValue(input)) {
      const auto isZero = model::isZero(input);
      n0 += (isZero ? 1 : 0);
      n1 += (isZero ? 0 : 1);
    } else {
      const auto i = oldToNewGates.find(input.node());
      assert(i != oldToNewGates.end());

      const auto newInputId = i->second;
      newInputs.push_back(Gate::Signal::always(newInputId));
    }
  }

  return newInputs;
}

Gate::Id MigMapper::mapGate(const Gate &oldGate,
                            const GateIdMap &oldToNewGates,
                            GNet &newNet) const {
  if (oldGate.isSource() || oldGate.isTrigger()) {
    // Clone sources and triggers gates w/o changes.
    return PreMapper::mapGate(oldGate, oldToNewGates, newNet);
  }

  size_t n0, n1;
  auto newInputs = getNewInputs(oldGate, oldToNewGates, n0, n1);

  switch (oldGate.func()) {
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
  case GateSymbol::MAJ  : return mapMaj(newInputs, n0, n1,        newNet);
  default: assert(false && "Unknown gate");
  }

  return Gate::INVALID;
}

//===----------------------------------------------------------------------===//
// ZERO/NOT(ZERO)
//===----------------------------------------------------------------------===//

Gate::Id MigMapper::mapVal(bool value, GNet &newNet) const {
  const auto gateId = newNet.addGate(GateSymbol::ZERO, {});
  if (value){
      return newNet.addGate(GateSymbol::NOT, {Gate::Signal::always(gateId)});
  }
  return gateId;
}

//===----------------------------------------------------------------------===//
// NOP/NOT
//===----------------------------------------------------------------------===//

Gate::Id MigMapper::mapNop(const Gate::SignalList &newInputs,
                           bool sign, GNet &newNet) const {
  // NOP(x) = x.
  const auto inputId = newInputs.at(0).node();
  if (sign) {
    return inputId;
  }

  // NOT(NOT(x)) = x.
  const auto *inputGate = Gate::get(inputId);
  if (inputGate->func() == GateSymbol::NOT) {
    return inputGate->input(0).node();
  }

  // NOT(x).
  return newNet.addGate(GateSymbol::NOT, newInputs);
}

Gate::Id MigMapper::mapNop(const Gate::SignalList &newInputs,
                           size_t n0, size_t n1, bool sign, GNet &newNet) const {
  assert(newInputs.size() + n0 + n1 == 1);

  if (n0 > 0 || n1 > 0) {
    return mapVal((n0 > 0) ^ sign, newNet);
  }

  return mapNop(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// AND/NAND
//===----------------------------------------------------------------------===//

Gate::Id MigMapper::mapAnd(const Gate::SignalList &newInputs,
                           bool sign, GNet &newNet) const {
  Gate::SignalList inputs(newInputs.begin(), newInputs.end());
  inputs.reserve(2 * newInputs.size() - 1);
  const auto valId = mapVal(false, newNet);

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
      gateId = newNet.addGate(GateSymbol::MAJ, {x, y, valId});
    }

    inputs.push_back(Gate::Signal::always(gateId));

    l += 2;
    r += 2;
  }

  return mapNop({inputs[l]}, sign, newNet);
}

Gate::Id MigMapper::mapAnd(const Gate::SignalList &newInputs,
                           size_t n0, size_t n1, bool sign, GNet &newNet) const {
  if (n0 > 0) {
    return mapVal(!sign, newNet);
  }

  return mapAnd(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// OR/NOR
//===----------------------------------------------------------------------===//

Gate::Id MigMapper::mapOr(const Gate::SignalList &newInputs,
                          bool sign, GNet &newNet) const {
    Gate::SignalList inputs(newInputs.begin(), newInputs.end());
    inputs.reserve(2 * newInputs.size() - 1);
    const auto valId = mapVal(true, newNet);

    size_t l = 0;
    size_t r = 1;
    while (r < inputs.size()) {
      const auto x = inputs[l];
      const auto y = inputs[r];

      Gate::Id gateId;
      if (model::areIdentical(x, y)) {
        // OR(x,x) = x.
        gateId = mapNop({x}, sign, newNet);
      } else if (model::areContrary(x, y)) {
        // OR(x,NOT(x)) = 1.
        gateId = mapVal(sign, newNet);
      } else {
        // OR(x,y).
        gateId = newNet.addGate(GateSymbol::MAJ, {x, y, valId});
      }

      inputs.push_back(Gate::Signal::always(gateId));

      l += 2;
      r += 2;
    }

    return mapNop({inputs[l]}, sign, newNet);
}

Gate::Id MigMapper::mapOr(const Gate::SignalList &newInputs,
                          size_t n0, size_t n1, bool sign, GNet &newNet) const {
  if (n1 > 0) {
    return mapVal(sign, newNet);
  }

  return mapOr(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// XOR/XNOR
//===----------------------------------------------------------------------===//

Gate::Id MigMapper::mapXor(const Gate::SignalList &newInputs,
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

Gate::Id MigMapper::mapXor(const Gate::SignalList &newInputs,
                           size_t n0, size_t n1, bool sign, GNet &newNet) const {
  if (n1 > 1) {
    return mapXor(newInputs, sign ^ (n1 & 1), newNet);
  }

  return mapXor(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// MAJ
//===----------------------------------------------------------------------===//

size_t factorial(size_t a) {
  if (a == 0 || a == 1) {
    return 1;
  }
  size_t result = 1;
  for (size_t i = 2; i <= a; ++i) {
    result *= i;
  }
  return result;
}

Gate::Id MigMapper::mapMaj(const Gate::SignalList &newInputs, size_t n0, size_t n1, GNet &newNet) const {
  assert(((newInputs.size() + n0 + n1) % 2 == 1) and (newInputs.size() + n0 + n1 >= 3));

  if (newInputs.size() == 0) {
    return mapVal((n1 > n0), newNet);
  }

  if ((newInputs.size() <= 3) and (n0 == n1)) {
    if (newInputs.size() == 3) {
      return newNet.addGate(GateSymbol::MAJ, newInputs);
    } else { //newInputs.size() == 1
      return mapNop(newInputs, true, newNet);
    }
  }

  int n = n1 - n0;
  size_t abs_n = abs(n);
  if ((newInputs.size() == 2) and (abs_n == 1)) {
    // Maj(x1, x2, 1) =  Or(x1, x2)
    // Maj(x1, x2, 0) = And(x1, x2)
    if (n1 > 0) {
      return mapOr(newInputs, true, newNet);
    } else {
      return mapAnd(newInputs, true, newNet);
    }
  }

  // med - is a mediana
  size_t med = (newInputs.size() + n0 + n1 + 1) / 2;
  if (n0 >= med) {
    return mapVal(false, newNet);
  }
  if (n1 >= med) {
    return mapVal(true, newNet);
  }

  Gate::SignalList inputs(newInputs.begin(), newInputs.end());
  const size_t quantity = newInputs.size() + abs_n;
  if (n != 0) {
    inputs.reserve(quantity);
    const auto valId = mapVal((n > 0), newNet);
    while (abs_n > 0) {
      inputs.push_back(Gate::Signal::always(valId));
      abs_n -= 1;
    }
  }
  // MAJ(x, y, z) = OR(AND(x, y), AND(y, z), AND(x, z))
  // MAJ(x, y, z, v, w) = OR(AND(x, y, z), AND(x, y, v), AND(x, y, w), AND(x, z, v), AND(x, z, w),
  //                         AND(x, v, w), AND(y, z, v), AND(y, z, w), AND(y, v, w), AND(z, v, w))
  // and so on...
  size_t C = factorial(quantity) / (factorial(med) * factorial(quantity - med));
  Gate::SignalList newestInputs;
  newestInputs.reserve(C);
  size_t *indexes = new size_t [med];
  for (size_t i = 0; i < med; ++i) {
    indexes[i] = i;
  }
  for (size_t i = 0; i < C; ++i) {
    int flag = 0;
    bool fl = true;
    for (size_t j = med; j > 0; --j) {
      indexes[j - 1] += flag;
      fl = true;
      if (indexes[j - 1] > quantity - 1 - (med - j)) {
        flag = 1;
        fl = false;
      }
      if (fl) {
        for (size_t k = j; k < med; ++k) {
          indexes[k] = indexes[k - 1] + 1;
        }
        break;
      }
    }
    Gate::SignalList tmpInputs;
    tmpInputs.reserve(med);
    for (size_t h = 0; h < med; ++h) {
      tmpInputs.push_back(inputs[indexes[h]]);
    }
    const auto gateAndId = mapAnd(tmpInputs, true, newNet);
    newestInputs.push_back(Gate::Signal::always(gateAndId));
    tmpInputs.clear();
    indexes[med - 1] += 1;
  }
  delete [] indexes;
  return mapOr(newestInputs, true, newNet);
}

} // namespace eda::gate::premapper
