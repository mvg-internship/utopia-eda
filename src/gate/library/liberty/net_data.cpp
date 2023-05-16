//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/library/liberty/net_data.h"
#include "gate/simulator/simulator.h"

#include <cassert>
#include <unordered_map>

namespace GModel = eda::gate::model;

using eda::gate::model::Gate;
using eda::gate::model::GateSymbol;
using eda::gate::simulator::Simulator;
using eda::gate::optimizer::RWDatabase;

static std::shared_ptr<GModel::GNet> makeNet(
    GateSymbol gate,
    unsigned N,
    Gate::SignalList &inputs,
    Gate::Id &outputId) {
  auto net = std::make_shared<GModel::GNet>();
  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->addIn();
    inputs.push_back(Gate::Signal::always(inputId));
  }
  auto gateId = net->addGate(gate, inputs);
  outputId = net->addOut(gateId);
  net->sortTopologically();
  return net;
}

void NetData::fillDatabase(RWDatabase &database) {
  std::unordered_map<RWDatabase::TruthTable, RWDatabase::BoundGNetList> storage;
  for (auto &net: combNets) {
    std::uint64_t N = net->nSourceLinks();
    Gate::SignalList inputs1;
    Gate::Id outputId1;
    auto newNet = makeNet(GateSymbol::AND, N, inputs1, outputId1);
    RWDatabase::BoundGNet bounder;
    std::uint64_t id = 0;
    for (auto link: newNet->sourceLinks()) {
      bounder.inputsDelay.emplace(id, 1);
      bounder.bindings.emplace(id, link.target);
      ++id;
    }
    bounder.net.reset(newNet.get());
    RWDatabase::TruthTable key = NetData::buildTruthTab(net.get())[0];
    storage[key].push_back(bounder);
  }
  for (auto &it: storage) {
    database.set(it.first, it.second);
  }
}

std::vector<RWDatabase::TruthTable> NetData::buildTruthTab(
    const GModel::GNet *net) {
  Gate::LinkList in, out;

  // NOTICE: each link is assumed to be one-bit-wide
  for (auto link: net->sourceLinks()) {
    in.push_back(Gate::Link(link.target));
  }

  for (auto link: net->targetLinks()) {
    out.push_back(Gate::Link(link.source));
  }

  static Simulator simulator;
  auto compilationResult = simulator.compile(*net, in, out);

  // Truth tables for each output
  std::vector<RWDatabase::TruthTable> outputsTruthTables(out.size(), 0);
  // The max value that can be applied via inputs.
  // FIXME: what about the absence of sources?
  assert(net->nSourceLinks() > 0);

  const uint64_t _1 = 1;
  const uint64_t maxInputValue = _1 << net->nSourceLinks();

  // NOTICE: it is assumed that each GNet has exactly 6 inputs.
  for (uint64_t inputValue = 0; inputValue < 64; ++inputValue) {
    RWDatabase::TruthTable plainTruthTable;
    // Calcaculate the truth table for the given input combination.
    compilationResult.simulate(plainTruthTable, inputValue % maxInputValue);

    // The truth table should be representable for the outputs.
    assert(plainTruthTable < (_1 << out.size()));

    // Save the truth table at the correspondent positions at outputs.
    for (uint64_t outSelector = 0; outSelector < out.size(); ++outSelector) {
      // Value bits for each output is arranged according to the combination
      // of inputs (i=0 leads to the 0th position, etc.).
      outputsTruthTables[outSelector] |=
        ((plainTruthTable >> outSelector) & 0x1) << inputValue;
    }
  }

  return outputsTruthTables;
}
