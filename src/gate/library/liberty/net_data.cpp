//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/library/liberty/net_data.h"
#include "gate/simulator/simulator.h"

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
  for (auto link: net->sourceLinks()) {
    in.push_back(Gate::Link(link.target));
  }
  for (auto link: net->targetLinks()) {
    out.push_back(Gate::Link(link.source));
  }
  static Simulator simulator;
  auto compiled = simulator.compile(*net, in, out);
  std::vector<RWDatabase::TruthTable> tableMean(out.size(), 0);
  uint64_t length = 1 << net->nSourceLinks();
  for (uint64_t i = 0; i < length; ++i) {
    RWDatabase::TruthTable mean;
    compiled.simulate(mean, i);
    for (uint64_t j = 0; j < tableMean.size(); ++j) {
      tableMean[j] += ((mean >> j) % 2) << i;
    }
  }
  return tableMean;
}
