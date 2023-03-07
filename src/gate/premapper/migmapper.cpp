//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/migmapper.h"

#include <cassert>
#include <unordered_set>

using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;

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
      assert((i != oldToNewGates.end()) && "Input node at the end\n");

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

  size_t n0;
  size_t n1;
  auto newInputs = getNewInputs(oldGate, oldToNewGates, n0, n1);

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
  case GateSymbol::MAJ  : return mapMaj(newInputs, n0, n1,        newNet);
  default: assert(false && "Unknown gate\n");
  }

  return Gate::INVALID;
}

//===----------------------------------------------------------------------===//
// IN/OUT
//===----------------------------------------------------------------------===//

Gate::Id MigMapper::mapIn(GNet &newNet) const {
  return newNet.addIn();
}

Gate::Id MigMapper::mapOut(const Gate::SignalList &newInputs,
                           size_t n0, size_t n1, GNet &newNet) const {
  assert(newInputs.size() + n0 + n1 == 1 && "Input size error of mapOut\n");

  // Constant output.
  if (n0 > 0 || n1 > 0) {
    auto valId = mapVal(n1 > 0, newNet);
    return newNet.addOut(valId);
  }

  return newNet.addOut(newInputs);
}

//===----------------------------------------------------------------------===//
// ZERO/NOT(ZERO)
//===----------------------------------------------------------------------===//

Gate::Id MigMapper::mapVal(bool value, GNet &newNet) const {
  const auto gateId = newNet.addZero();
  if (value) {
    return newNet.addNot(Gate::Signal::always(gateId));
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
  return newNet.addNot(newInputs.at(0));
}

Gate::Id MigMapper::mapNop(const Gate::SignalList &newInputs,
                           size_t n0, size_t n1,
                           bool sign, GNet &newNet) const {
  assert((newInputs.size() + n0 + n1 == 1) && "Too many sources for nop\n");

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
  const auto valId = newNet.addZero();

  size_t left = 0;
  size_t right = 1;
  while (right < inputs.size()) {
    const auto x = inputs[left];
    const auto y = inputs[right];

    Gate::Id gateId;
    if (model::areIdentical(x, y)) {
      // AND(x,x) = x.
      gateId = mapNop({x}, sign, newNet);
    } else if (model::areContrary(x, y)) {
      // AND(x,NOT(x)) = 0.
      gateId = mapVal(!sign, newNet);
    } else {
      // AND(x,y).
      gateId = newNet.addGate(GateSymbol::MAJ, 
                             {x, y, Gate::Signal::always(valId)});
    }

    inputs.push_back(Gate::Signal::always(gateId));

    left += 2;
    right += 2;
  }

  return mapNop({inputs[left]}, sign, newNet);
}

Gate::Id MigMapper::mapAnd(const Gate::SignalList &newInputs,
                           size_t n0, size_t n1, 
                           bool sign, GNet &newNet) const {
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

    size_t left = 0;
    size_t right = 1;
    while (right < inputs.size()) {
      const auto x = inputs[left];
      const auto y = inputs[right];

      Gate::Id gateId;
      if (model::areIdentical(x, y)) {
        // OR(x,x) = x.
        gateId = mapNop({x}, sign, newNet);
      } else if (model::areContrary(x, y)) {
        // OR(x,NOT(x)) = 1.
        gateId = mapVal(sign, newNet);
      } else {
        // OR(x,y).
        gateId = newNet.addGate(GateSymbol::MAJ,
                               {x, y, Gate::Signal::always(valId)});
      }

      inputs.push_back(Gate::Signal::always(gateId));

      left += 2;
      right += 2;
    }

    return mapNop({inputs[left]}, sign, newNet);
}

Gate::Id MigMapper::mapOr(const Gate::SignalList &newInputs,
                          size_t n0, size_t n1,
                          bool sign, GNet &newNet) const {
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

  size_t left = 0;
  size_t right = 1;
  while (right < inputs.size()) {
    // XOR (x,y)=AND(NAND(x,y),NAND(NOT(x),NOT(y))): 7 AND and NOT gates.
    // XNOR(x,y)=AND(NAND(x,NOT(y)),NAND(NOT(x),y)): 7 AND and NOT gates.
    const auto x1 = mapNop({inputs[left]}, true, newNet);
    const auto y1 = mapNop({inputs[right]}, sign, newNet);
    const auto x2 = mapNop({inputs[left]}, false, newNet);
    const auto y2 = mapNop({inputs[right]}, !sign, newNet);

    const auto z1 = mapAnd({Gate::Signal::always(x1), 
                            Gate::Signal::always(y1)},
                            false, newNet);
    const auto z2 = mapAnd({Gate::Signal::always(x2),
                            Gate::Signal::always(y2)},
                            false, newNet);
    const auto id = mapAnd({Gate::Signal::always(z1),
                            Gate::Signal::always(z2)},
                            true, newNet);

    inputs.push_back(Gate::Signal::always(id));

    left += 2;
    right += 2;

    sign = true;
  }

  return inputs[left].node();
}

Gate::Id MigMapper::mapXor(const Gate::SignalList &newInputs,
                           size_t n0, size_t n1,
                           bool sign, GNet &newNet) const {
  if (n1 > 1) {
    return mapXor(newInputs, sign ^ (n1 & 1), newNet);
  }

  return mapXor(newInputs, sign, newNet);
}

//===----------------------------------------------------------------------===//
// MAJ
//===----------------------------------------------------------------------===//

Gate::Id majorityOfFive(const Gate::SignalList &newInputs, GNet &newNet) {
  // <xyztu> = <<xyz>t<<xyu>uz>>
  const auto xyzId = newNet.addGate(GateSymbol::MAJ,
                                   {newInputs[0],
                                    newInputs[1], 
                                    newInputs[2]});
  const auto xyuId = newNet.addGate(GateSymbol::MAJ,
                                   {newInputs[0],
                                    newInputs[1],
                                    newInputs[4]});
  // <<xyu>uz>
  const auto muzId = newNet.addGate(GateSymbol::MAJ,
                                   {newInputs[4],
                                    newInputs[2],
                                    Gate::Signal::always(xyuId)});
  return newNet.addGate(GateSymbol::MAJ,
                       {Gate::Signal::always(xyzId), 
                        newInputs[3],
                        Gate::Signal::always(muzId)});
}

Gate::Id majorityOfSeven(const Gate::SignalList &newInputs, GNet &newNet) {
  // <xyztufr> = <y<u<xzt><NOT(u)fr>><t<ufr><xzNOT(t)>>>
  const auto xztId = newNet.addGate(GateSymbol::MAJ,
                                   {newInputs[0],
                                    newInputs[2],
                                    newInputs[3]});
  const auto notUId = newNet.addNot(newInputs[4]);
  const auto notUfrId = newNet.addGate(GateSymbol::MAJ,
                                      {newInputs[5],
                                       newInputs[6],
                                       Gate::Signal::always(notUId)});
  const auto ufrId = newNet.addGate(GateSymbol::MAJ,
                                   {newInputs[4],
                                    newInputs[5], 
                                    newInputs[6]});
  const auto notTId = newNet.addNot(newInputs[3]);
  const auto xzNotTId = newNet.addGate(GateSymbol::MAJ,
                                      {newInputs[0],
                                       newInputs[2],
                                       Gate::Signal::always(notTId)});
  // <u<xzt><NOT(u)fr>>
  const auto uxztufrId = newNet.addGate(GateSymbol::MAJ,
                                       {newInputs[4],
                                        Gate::Signal::always(xztId),
                                        Gate::Signal::always(notUfrId)});
  // <t<ufr><xzNOT(t)>>
  const auto tufrxztId = newNet.addGate(GateSymbol::MAJ,
                                       {newInputs[3],
                                        Gate::Signal::always(ufrId),
                                        Gate::Signal::always(xzNotTId)});
  // <xyztufr>
  return newNet.addGate(GateSymbol::MAJ,
                       {newInputs[1], 
                        Gate::Signal::always(uxztufrId), 
                        Gate::Signal::always(tufrxztId)});
}

Gate::Id MigMapper::mapMaj(const Gate::SignalList &newInputs, size_t n0, 
                           size_t n1, GNet &newNet) const {
  size_t inputSize = newInputs.size();
  assert(((inputSize + n0 + n1) % 2 == 1) && (inputSize + n0 + n1 >= 3)
                                          && "Invalid number of inputs\n");

  if (inputSize == 0) {
    return mapVal((n1 > n0), newNet);
  }

  if ((inputSize <= 3) && (n0 == n1)) {
    if (inputSize == 3) {
      return newNet.addGate(GateSymbol::MAJ, newInputs);
    }
    // inputSize == 1
    return mapNop(newInputs, true, newNet);
  }

  int n = n1 - n0;
  size_t absN = abs(n);
  if ((inputSize == 2) && (absN == 1)) {
    // Maj(x1, x2, 1) =  Or(x1, x2)
    // Maj(x1, x2, 0) = And(x1, x2)
    if (n1 > 0) {
      return mapOr(newInputs, true, newNet);
    }
    return mapAnd(newInputs, true, newNet);
  }

  size_t median = (inputSize + n0 + n1 + 1) / 2;
  if (n0 >= median) {
    return mapVal(false, newNet);
  }
  if (n1 >= median) {
    return mapVal(true, newNet);
  }

  Gate::SignalList inputs(newInputs.begin(), newInputs.end());
  const size_t quantity = inputSize + absN;
  if (n != 0) {
    inputs.reserve(quantity);
    const auto valId = mapVal((n > 0), newNet);
    for (absN; absN > 0; absN--) {
      inputs.push_back(Gate::Signal::always(valId));
    }
  }
  if (quantity == 5) {
    return majorityOfFive(inputs, newNet);
  }
  if (quantity == 7) {
    return majorityOfSeven(inputs, newNet);
  }
  const size_t halfQuan = (quantity + 1) / 2;

  // Majority of n to several majority of 3
  Gate::SignalList toFitIn(halfQuan);
  Gate::SignalList toTakeFrom(halfQuan);
  const size_t nLastArg = quantity - 1;
  const size_t nPenulArg = quantity - 2;
  const size_t nBefPenulArg = quantity - 3;
  const auto notArgId = newNet.addNot(inputs[nBefPenulArg]);
  const auto baseMaj1 = newNet.addGate(GateSymbol::MAJ,
                                      {Gate::Signal::always(notArgId),
                                       inputs[nPenulArg],
                                       inputs[nLastArg]});
  const auto baseMaj2 = newNet.addGate(GateSymbol::MAJ,
                                      {inputs[nBefPenulArg],
                                       inputs[nPenulArg],
                                       inputs[nLastArg]});
  toTakeFrom[1] = Gate::Signal::always(baseMaj2);
  const auto zeroId = newNet.addZero();
  const auto oneId  = newNet.addNot(Gate::Signal::always(zeroId));
  const auto startMaj1 = newNet.addGate(GateSymbol::MAJ,
                                       {inputs[nBefPenulArg],
                                        Gate::Signal::always(zeroId),
                                        Gate::Signal::always(baseMaj1)});
  const auto startMaj2 = newNet.addGate(GateSymbol::MAJ,
                                       {inputs[nBefPenulArg],
                                        Gate::Signal::always(oneId),
                                        Gate::Signal::always(baseMaj1)});
  toTakeFrom[0] = Gate::Signal::always(startMaj1);
  toTakeFrom[2] = Gate::Signal::always(startMaj2);
  // the layer size
  size_t layerSize = 4;
  // the lower part of the rhombus
  for (size_t i = nBefPenulArg; i >= halfQuan; i--) {
    // begin outer cycle
    // the cycle for a layer
    const auto firstMaj = newNet.addGate(GateSymbol::MAJ,
                                        {inputs[i - 1],
                                         Gate::Signal::always(zeroId),
                                         toTakeFrom[0]});
    toFitIn[0] = Gate::Signal::always(firstMaj);
    for (size_t j = 1; j < layerSize - 1; j++) {
      // begin inner cycle
      const auto intermedMaj = newNet.addGate(GateSymbol::MAJ,
                                             {inputs[i - 1],
                                              toTakeFrom[j - 1],
                                              toTakeFrom[j]});
      toFitIn[j] = Gate::Signal::always(intermedMaj);
    } // end inner cycle
    const auto lastMaj = newNet.addGate(GateSymbol::MAJ,
                                       {inputs[i - 1],
                                        Gate::Signal::always(oneId),
                                        toTakeFrom[layerSize - 2]});
    toFitIn[layerSize - 1] = Gate::Signal::always(lastMaj);
    layerSize++;
    std::swap(toFitIn, toTakeFrom);
    toFitIn.clear();
  } // end outer cycle
  // one more counter
  size_t counter;
  // the higher part of the rhombus
  for (counter = (nBefPenulArg) / 2; counter > 1; counter -= 2) {
    // begin outer cycle
    // the cycle for a layer
    for (size_t j = 0; j < counter; j++) {
      // begin inner cycle
      const auto internalMaj = newNet.addGate(GateSymbol::MAJ,
                                             {inputs[counter - 1],
                                              toTakeFrom[j],
                                              toTakeFrom[j + 2]});
      const auto externalMaj = newNet.addGate(GateSymbol::MAJ,
                                             {inputs[counter],
                                              toTakeFrom[j + 1],
                                           Gate::Signal::always(internalMaj)});
      toFitIn[j] = Gate::Signal::always(externalMaj);
    } // end inner cycle
    std::swap(toFitIn, toTakeFrom);
    toFitIn.clear();
  } // end outer cycle
  if (counter == 1) {
    const auto internalMaj = newNet.addGate(GateSymbol::MAJ,
                                           {inputs[0],
                                            toTakeFrom[0],
                                            toTakeFrom[2]});
    return newNet.addGate(GateSymbol::MAJ,
                         {inputs[1], 
                          toTakeFrom[1],
                          Gate::Signal::always(internalMaj)});
  }
  return newNet.addGate(GateSymbol::MAJ,
                       {inputs[0],
                        toTakeFrom[0],
                        toTakeFrom[1]});
}

} // namespace eda::gate::premapper