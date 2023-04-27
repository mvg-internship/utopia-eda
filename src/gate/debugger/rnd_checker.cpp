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
        std::uint64_t in = temp % static_cast<std::uint64_t>(std::pow(2, count));
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
    for (std::uint64_t t = 0; t < std::pow(2, count); t++) {
      std::uint64_t temp = 2*t;
      std::uint64_t in = temp % static_cast<std::uint64_t>(std::pow(2, count));
      compiled.simulate(o, in);
      if (o == 1) {
        return  Result::NOTEQUAL;
      }
    }
    return Result::EQUAL;
  }

  return Result::ERROR;
}

void RNDChecker::setTries(int tries) {
  this->tries = tries;
}

void RNDChecker::setExhaustive(bool exhaustive) {
  this->exhaustive = exhaustive;
}

bool RNDChecker::areEqual(GNet &lhs,
                            GNet &rhs,
                            Checker::GateIdMap &gmap) {

    GateBinding ibind, obind, tbind;

  // Input-to-input correspondence.
  for (auto oldSourceLink : lhs.sourceLinks()) {
    auto newSourceId = gmap[oldSourceLink.target];
    ibind.insert({oldSourceLink, Gate::Link(newSourceId)});
  }

  // Output-to-output correspondence.
  for (auto oldTargetLink : lhs.targetLinks()) {
    auto newTargetId = gmap[oldTargetLink.source];
    obind.insert({oldTargetLink, Gate::Link(newTargetId)});
  }

  // Trigger-to-trigger correspondence.
  for (auto oldTriggerId : lhs.triggers()) {
    auto newTriggerId = gmap[oldTriggerId];
    tbind.insert({Gate::Link(oldTriggerId), Gate::Link(newTriggerId)});
  }

  Checker::Hints hints;
  hints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tbind));

    GNet *net = miter(lhs, rhs, hints);
    Result res = Generator(*net, tries, exhaustive);
    return (res == 0);
  }

} // namespace eda::gate::debugger