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

static simulator::Simulator simulator;

Result Generator(GNet &miter, const unsigned int tries, const bool exhaustive = true) {

  //check the number of output
  assert(miter.nTargetLinks() == 1);

  std::uint64_t count = miter.sourceLinks().size();

  // counting the inputs
  //for (auto x : miter.sourceLinks()) {
  //count += 1;
  //}

  GNet::In inp(1);
  auto &input = inp[0];

  for (auto x : miter.sourceLinks()) {
     input.push_back(x.target);
  }

  Gate::SignalList inputs;
  Gate::Id output = 0;
  GNet::LinkList in;

  for (size_t n = 0; n < count; n++) {
    in.push_back(GNet::Link(input[n]));
  }

  GNet::LinkList out{Gate::Link(output)};

  for (auto input : inputs) {
    in.push_back(GNet::Link(input.node()));
  }

  miter.sortTopologically();
  auto compiled = simulator.compile(miter, in, out);
  std::uint64_t o;

  if (!exhaustive) {
  // UNexhaustive check
    for (std::uint64_t t = 0; t < tries; t++) {
      for (std::uint64_t i = 0; i < count; i++) {
        std::uint64_t temp = 2*rand();
        std::uint64_t in = temp % static_cast<std::uint64_t>(std::pow(2, count - 1));
        compiled.simulate(o, in);
        if (o == 1) {
          return  Result::NOTEQUAL;
        }
      }
    }
    return Result::UNKNOWN;
  }

  if (exhaustive) {
  // exhaustive check
    for (std::uint64_t t = 0; t < std::pow(2, count - 1); t++) {
      std::uint64_t temp = 2*t;
      std::uint64_t in = temp % static_cast<std::uint64_t>(std::pow(2, count - 1));
      compiled.simulate(o, in);
      if (o == 1) {
        return  Result::NOTEQUAL;
      }
    }
    return Result::EQUAL;
  }

  return Result::ERROR;
}

} // namespace eda::gate::debugger
