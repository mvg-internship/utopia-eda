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
using eda::gate::simulator::Simulator;
using eda::gate::optimizer::RWDatabase;

void NetData::fillDatabase(RWDatabase &database) {
  RWDatabase::InputId id = 0;
  std::unordered_map<RWDatabase::TruthTable, RWDatabase::BoundGNetList> storage;
  for (auto &net: combNets) {
    RWDatabase::BoundGNet bounder;
    for (auto link: net->sourceLinks()) {
      bounder.bindings.emplace(id++, link.target);
    }
    RWDatabase::TruthTable key = NetData::buildTruthTab(net.get())[0];
    bounder.net.reset(net.release());
    storage[key].push_back(bounder);
  }
  for (auto &it: storage) {
    database.set(it.first, it.second);
  }
}

///Returns vector of means, where vector[0] is mean for 1st out of Gnet and etc.
std::vector<RWDatabase::TruthTable> NetData::buildTruthTab(
    const GModel::GNet *net) {
  static Simulator simulator;
  Gate::LinkList in, out;
  for (auto link: net->sourceLinks()) {
    in.push_back(Gate::Link(link.target));
  }
  for (auto link: net->targetLinks()) {
    out.push_back(Gate::Link(link.source));
  }
  auto compiled = simulator.compile(*net, in, out);
  RWDatabase::TruthTable mean;
  std::vector<RWDatabase::TruthTable> tMean;
  tMean.resize(out.size());
  std::fill(tMean.begin(), tMean.end(), 0);
  uint64_t length = 1 << net->nSourceLinks();
  for (uint64_t i = 0; i < length; ++i) {
    compiled.simulate(mean, i);
    for (uint64_t j = 0; j < tMean.size(); ++j) {
      tMean[j] += ((mean >> j) % 2) << i;
    }
  }
  return tMean;
}
