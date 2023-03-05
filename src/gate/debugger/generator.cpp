//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "gate/simulator/simulator.h"
#include "rtl/library/flibrary.h"
#include <cassert>
#include <cmath>

using GNet = eda::gate::model::GNet;

enum Result {
  ERROR = -2,
  UNKNOW = -1,
  EQUAL = 0,
  NOTEQUAL = 1,
};

Result Generator(const GNet &net, const unsigned int tries, bool flag = false) {

  //check the number of output
  assert(net.nTargetLinks() == 1);

  std::uint64_t count = 0;

  // counting the number of inputs
  for (auto x : net.sourceLinks()) {
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
  auto compiled = simulator.compile(net, in, out);


  std::uint64_t o;
  if (!flag) {
    for (std::uint64_t t = 0; t < tries; t++) {
      for (std::uint64_t i = 0; i < count; i++) {
        compiled.simulate(o, i);
        if (o == 1) {
          return  Result::NOTEQUAL;
        }
      }
    }
  return Result::UNKNOW;
  }

  if (flag) {
    for (std::uint64_t t = 0; t < std::pow(2, net.nSourceLinks()); t++) {
      for (std::uint64_t i = 0; i < count; i++) {
        compiled.simulate(o, i);
        if (o == 1) {
          return  Result::NOTEQUAL;
        }
      }
    }
  return Result::EQUAL;
  }

  return Result::ERROR;
}
