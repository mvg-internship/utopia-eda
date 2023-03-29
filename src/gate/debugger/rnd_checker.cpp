//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/rnd_checker.h"

using GNet = eda::gate::model::GNet;

namespace eda::gate::debugger {

Result Generator(const GNet* miter, const unsigned int tries, bool flag = true) {

  //check the number of output
  assert(miter->nTargetLinks() == 1);

  std::uint64_t count = 0;

  // counting the summury arity of inputs
  for (auto x : miter->sourceLinks()) {
    count += Gate::get(x.source)->arity();
  }

  GNet::SignalList inputs;
  GNet::GateId output = 0;

  GNet::LinkList in;
  GNet::LinkList out{Gate::Link(output)};

    for (auto input : inputs) {
    in.push_back(Gate::Link(input.node()));
  }

  eda::gate::simulator::Simulator simulator;
  auto compiled = simulator.compile(*miter, in, out);

  std::uint64_t o;
  if (!flag) {
  // UNexhaustive check
    for (std::uint64_t t = 0; t < tries; t++) {
      for (std::uint64_t i = 0; i < count; i++) {
        std::uint64_t in = rand();
        compiled.simulate(o, in);
        if (o == 1) {
          return  Result::NOTEQUAL;
        }
      }
    }
    return Result::UNKNOWN;
  }

  if (flag) {
  // exhaustive check
    for (std::uint64_t t = 0; t < std::pow(2, count); t++) {
      compiled.simulate(o, t);
      if (o == 1) {
        return  Result::NOTEQUAL;
      }
    }
    return Result::EQUAL;
  }

  return Result::ERROR;
}
} // namespace eda::gate::debugger
