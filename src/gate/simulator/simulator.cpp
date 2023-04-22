//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/simulator/simulator.h"
#include "util/assert.h"

namespace eda::gate::simulator {

using Compiled = Simulator::Compiled;

Compiled::OP Compiled::getOp(const Gate &gate) const {
  using GateSymbol = eda::gate::model::GateSymbol;

  const auto n = gate.arity();

  switch (gate.func()) {
  case GateSymbol::OUT   : return getNop(n);
  case GateSymbol::ZERO  : return getZero(n);
  case GateSymbol::ONE   : return getOne(n);
  case GateSymbol::NOP   : return getNop(n);
  case GateSymbol::NOT   : return getNot(n);
  case GateSymbol::AND   : return getAnd(n);
  case GateSymbol::OR    : return getOr(n);
  case GateSymbol::XOR   : return getXor(n);
  case GateSymbol::NAND  : return getNand(n);
  case GateSymbol::NOR   : return getNor(n);
  case GateSymbol::XNOR  : return getXnor(n);
  case GateSymbol::LATCH : return getLatch(n);
  case GateSymbol::DFF   : return getDff(n);
  case GateSymbol::DFFrs : return getDffrs(n);
  default: uassert(false,
                   "Unsupported func symbol: " << gate.func() << std::endl);
  }

  return getZero(0);
}

Compiled::Command Compiled::getCommand(const GNet &net,
                                       const Gate &gate) const {
  const auto op = getOp(gate);

  const auto target = gate.id();
  Gate::Link outLink(target);
  const auto out = gindex.find(outLink)->second;

  IV in(gate.arity());
  for (I i = 0; i < gate.arity(); i++) {
    const auto source = gate.input(i).node();

    Gate::Link inLink = net.contains(source)
                      ? Gate::Link(source)
                      : Gate::Link(source, target, i);

    in[i] = gindex.find(inLink)->second;
  }

  return Command{op, out, in};
}

Compiled::Compiled(const GNet &net,
                   const GNet::LinkList &in,
                   const GNet::LinkList &out):
    program(net.nGates()),
    nInputs(in.size()),
    outputs(out.size()),
    memory(net.nSourceLinks() + net.nGates()),
    postponed(net.nTriggers()),
    nPostponed(0) {

  assert(net.isSorted() && "Net is not topologically sorted");
  assert(net.nSourceLinks() == in.size());

  gindex.reserve(net.nSourceLinks() + net.nGates());

  // Map the source links (including source gates) to memory.
  I i = 0;
  for (const auto link : in) {
    gindex[link] = i++;
  }

  // Map the non-source gates to memory.
  for (const auto *gate : net.gates()) {
    if (gate->isSource()) continue;
    Gate::Link link(gate->id());
    gindex[link] = i++;
  }

  // Determine the output indices.
  i = 0;
  for (const auto link : out) {
    outputs[i++] = gindex[link];
  }

  // Compose the simulation program.
  i = 0;
  for (const auto *gate : net.gates()) {
    if (gate->isSource()) continue;
    program[i++] = getCommand(net, *gate);
  }
  program.resize(i);
}

} // namespace eda::gate::simulator
