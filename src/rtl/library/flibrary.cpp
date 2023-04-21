//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "rtl/library/flibrary.h"

#include <cassert>

using namespace eda::base::model;
using namespace eda::gate::model;
using namespace eda::rtl::model;

namespace eda::rtl::library {

void fillWithZeros(const size_t size,
                   GNet::GateIdList &in,
                   GNet &net) {
  while (size > in.size()) {
    in.push_back(net.addZero());
  }
}

void makeInputsEqual(const size_t outSize,
                     GNet::GateIdList &x,
                     GNet::GateIdList &y,
                     GNet &net) {
  if ((outSize >= x.size()) && (outSize >= y.size())) {
    // Make x.size() and y.size() equal to the maximum of them
    fillWithZeros(x.size(), {y}, net);
    fillWithZeros(y.size(), {x}, net);
  } else {
    // Make x.size() and y.size() equal to the outSize
    fillWithZeros(outSize, {x}, net);
    fillWithZeros(outSize, {y}, net);
    x.resize(outSize);
    y.resize(outSize);
  }
}

GNet::GateIdList leftShiftForGateIdList(const GNet::GateIdList &x,
                                        const size_t shift,
                                        GNet &net) {
  GNet::GateIdList list(x.size() + shift);
  size_t i;
  for (i = 0; i < shift; i++) {
    list[i] = net.addZero();
  }
  for (; i < list.size(); i++) {
    list[i] = x[i - shift];
  }
  return list;
}

bool FLibraryDefault::supports(FuncSymbol func) const {
  return true;
}

FLibrary::Out FLibraryDefault::synth(size_t outSize,
                                     const Value &value,
                                     GNet &net) {
  assert(outSize == value.size());

  GNet::Out out(outSize);
  for (size_t i = 0; i < out.size(); i++) {
    out[i] = net.addGate((value[i] ? GateSymbol::ONE : GateSymbol::ZERO), {});
  }

  return out;
}

FLibrary::Out FLibraryDefault::synth(size_t outSize,
                                     const Out &out,
                                     GNet &net) {
  assert(outSize == out.size());

  GNet::Out targets(outSize);
  for (size_t i = 0; i < out.size(); i++) {
    targets[i] = net.addGate(GateSymbol::OUT, {Signal::always(out[i])});
  }

  return targets;
} 

FLibrary::Out FLibraryDefault::synth(size_t outSize,
                                     FuncSymbol func,
                                     const In &in,
                                     GNet &net) {
  switch (func) {
  case FuncSymbol::NOP:
    return synthUnaryBitwiseOp(GateSymbol::NOP, outSize, in, net);
  case FuncSymbol::NOT:
    return synthUnaryBitwiseOp(GateSymbol::NOT, outSize, in, net);
  case FuncSymbol::AND:
    return synthBinaryBitwiseOp(GateSymbol::AND, outSize, in, net);
  case FuncSymbol::OR:
    return synthBinaryBitwiseOp(GateSymbol::OR, outSize, in, net);
  case FuncSymbol::XOR:
    return synthBinaryBitwiseOp(GateSymbol::XOR, outSize, in, net);
  case FuncSymbol::ADD:
    return synthAdd(outSize, in, net);
  case FuncSymbol::SUB:
    return synthSub(outSize, in, net);
  case FuncSymbol::MUL:
    return synthMul(outSize, in, net);
  case FuncSymbol::MUX:
    return synthMux(outSize, in, net);
  default:
    assert(false);
    return {};
  }
}

FLibrary::Out FLibraryDefault::alloc(size_t outSize, GNet &net) {
  Out out(outSize);
  for (size_t i = 0; i < out.size(); i++) {
    out[i] = net.newGate();
  }

  return out;
}

FLibrary::Out FLibraryDefault::synth(const Out &out,
                                     const In &in,
                                     const SignalList &control,
                                     GNet &net) {
  assert(control.size() == 1 || control.size() == 2);
  assert(control.size() == in.size());

  auto clock = invertIfNegative(control[0], net);

  if (control.size() == 1) {
    const auto &x = in[0];
    assert(out.size() == x.size());

    for (size_t i = 0; i < out.size(); i++) {
      auto f = (clock.isEdge() ? GateSymbol::DFF : GateSymbol::LATCH);
      auto d = Signal::always(x[i]); // stored data

      net.setGate(out[i], f, { d, clock });
    }
  } else {
    const auto &x = in[0];
    const auto &y = in[1];
    assert(x.size() == y.size() && out.size() == x.size());

    auto edged = invertIfNegative(control[1], net);
    auto reset = Signal::always(edged.node());

    for (size_t i = 0; i < out.size(); i++) {
      auto d = Signal::always(x[i]); // stored data
      auto v = Signal::always(y[i]); // reset value
      auto n = Signal::always(net.addGate(GateSymbol::NOT, { v }));
      auto r = Signal::level1(net.addGate(GateSymbol::AND, { n, reset }));
      auto s = Signal::level1(net.addGate(GateSymbol::AND, { v, reset }));

      net.setGate(out[i], GateSymbol::DFFrs, { d, clock, r, s });
    }
  }

  // Return the given outputs.
  return out;
}

FLibrary::Out FLibraryDefault::synthAdd(size_t outSize,
                                        const In &in,
                                        GNet &net) {
   auto x = in[0];
   auto y = in[1];

   makeInputsEqual(outSize, x, y, net);

   return synthAdder(outSize, {x, y}, false, net);
}

FLibrary::Out FLibraryDefault::synthSub(size_t outSize,
                                        const In &in,
                                        GNet &net) {
  // The two's complement code: (x - y) == (x + ~y + 1).
  auto x = in[0];
  auto y = in[1];

  makeInputsEqual(outSize, x, y, net);

  Out temp = synthUnaryBitwiseOp(GateSymbol::NOT, y.size(), { y }, net);
  return synthAdder(outSize, { x, temp }, true, net);
}

FLibrary::Out FLibraryDefault::synthMul(size_t outSize,
                                        const In &in,
                                        GNet &net) {
  return synthMultiplier(outSize, in, net);
}

FLibrary::Out FLibraryDefault::synthAdder(size_t outSize,
                                          const In &in,
                                          bool plusOne,
                                          GNet &net) {
  assert(in.size() == 2);

  const auto &x = in[0];
  const auto &y = in[1];
  
  assert(x.size() == y.size());

  const bool needsCarryOut = ((outSize > x.size()) && (!plusOne));
  const size_t sumSize = needsCarryOut ? (x.size() + 1) : x.size();
  auto carryIn = plusOne ? net.addOne() : net.addZero();

  Out out(x.size());
  for (size_t i = 0; i < x.size(); i++) {
    const auto needsCarryOut = (i != sumSize - 1);
    auto zCarryOut = synthAdder(x[i], y[i], carryIn, needsCarryOut, net);

    out[i] = zCarryOut[0];
    carryIn = needsCarryOut ? zCarryOut[1] : Gate::INVALID;
  }

  if (carryIn != Gate::INVALID) {
    out.push_back(carryIn);
  }

  fillWithZeros(outSize, {out}, net);

  return out;
}

FLibrary::Out FLibraryDefault::synthAdder(Gate::Id x,
                                          Gate::Id y,
                                          Gate::Id carryIn,
                                          bool needsCarryOut,
                                          GNet &net) {
  auto xWire = Signal::always(x);
  auto yWire = Signal::always(y);
  auto cWire = Signal::always(carryIn);

  // {z, carryOut}.
  Out out;

  // z = (x + y) + carryIn (mod 2).
  auto xPlusY = Signal::always(net.addGate(GateSymbol::XOR, { xWire, yWire }));
  out.push_back(net.addGate(GateSymbol::XOR, { xPlusY, cWire }));

  if (needsCarryOut) {
    // carryOut = (x & y) | (x + y) & carryIn.
    auto carryOutLhs = Signal::always(net.addGate(GateSymbol::AND,
                                                  { xWire, yWire }));
    auto carryOutRhs = Signal::always(net.addGate(GateSymbol::AND,
                                                  { xPlusY, cWire }));
    out.push_back(net.addGate(GateSymbol::OR, { carryOutLhs, carryOutRhs }));
  }

  return out;
}

FLibrary::Out FLibraryDefault::synthMultiplier(const size_t outSize,
                                               const In &in,
                                               GNet &net) {
  const auto &x = in[0];
  const auto &y = in[1];

  size_t min1 = std::min(y.size(), outSize);
  size_t min2 = std::min(x.size(), outSize);

  auto out = synthMultiplier(min2, x, y[0], net);
  for (size_t i = 1; i < min1; i++) {
    min2 = std::min((x.size() + i), outSize);
    auto temp1 = synthMultiplier((min2 - i), x, y[i], net);
    GateIdList temp2 = leftShiftForGateIdList(temp1, i, net);
    size_t min3 = std::min((temp2.size() + 1), outSize);
    out = synthAdd(min3, {temp2, out}, net);
  }
  fillWithZeros(outSize, {out}, net);
  return out;
}

FLibrary::Out FLibraryDefault::synthMultiplier(const size_t outSize,
                                               const GateIdList &x,
                                               const GateId &y,
                                               GNet &net) {
  Out out;
  size_t mulSize = std::min(outSize, x.size());
  for (size_t i = 0; i < mulSize; i++) {
    out.push_back(net.addGate(GateSymbol::AND, x[i], y));
  }
  fillWithZeros(outSize, {out}, net);
  return out;
}

FLibrary::Out FLibraryDefault::synthMux(size_t outSize,
                                        const In &in,
                                        GNet &net) {
  assert(in.size() >= 4 && (in.size() & 1) == 0);
  const size_t n = in.size() / 2;

  Out out(outSize);
  for (size_t i = 0; i < out.size(); i++) {
    std::vector<Signal> temp;
    temp.reserve(n);

    for (size_t j = 0; j < n; j++) {
      const GateIdList &c = in[j];
      const GateIdList &x = in[j + n];
      assert(c.size() == 1 && out.size() == x.size());

      auto cj0 = Signal::always(c[0]);
      auto xji = Signal::always(x[i]);
      auto id = net.addGate(GateSymbol::AND, { cj0, xji });

      temp.push_back(Signal::always(id));
    }

    out[i] = net.addGate(GateSymbol::OR, temp);
  }

  return out;
}

FLibrary::Out FLibraryDefault::synthUnaryBitwiseOp(GateSymbol func,
                                                   size_t outSize,
                                                   const In &in,
                                                   GNet &net) {
  assert(in.size() == 1);

  const auto &x = in[0];
  assert(outSize == x.size());

  Out out(outSize);
  for (size_t i = 0; i < out.size(); i++) {
    auto xi = Signal::always(x[i]);
    out[i] = net.addGate(func, { xi });
  }

  return out;
}

FLibrary::Out FLibraryDefault::synthBinaryBitwiseOp(GateSymbol func,
                                                    size_t outSize,
                                                    const In &in,
                                                    GNet &net) {
  assert(in.size() == 2);

  const auto &x = in[0];
  const auto &y = in[1];
  assert(x.size() == y.size() && outSize == x.size());

  Out out(outSize);
  for (size_t i = 0; i < out.size(); i++) {
    auto xi = Signal::always(x[i]);
    auto yi = Signal::always(y[i]);
    out[i] = net.addGate(func, { xi, yi });
  }

  return out;
}


FLibrary::Signal FLibraryDefault::invertIfNegative(const Signal &event, GNet &net) {
  switch (event.event()) {
  case POSEDGE:
    // Leave the clock signal unchanged.
    return Signal::posedge(event.node());
  case NEGEDGE:
    // Invert the clock signal.
    return Signal::posedge(net.addGate(GateSymbol::NOT,
                                       { Signal::always(event.node()) }));
  case LEVEL0:
    // Invert the enable signal.
    return Signal::level1(net.addGate(GateSymbol::NOT,
                                      { Signal::always(event.node()) }));
  case LEVEL1:
    // Leave the enable signal unchanged.
    return Signal::level1(event.node());
  default:
    assert(false);
    return Signal::posedge(Gate::INVALID);
  }
}

} // namespace eda::rtl::library
