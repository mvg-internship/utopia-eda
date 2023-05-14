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
  std::unordered_map<RWDatabase::TruthTable, RWDatabase::BoundGNetList> storage;
  for (auto &net: combNets) {
    RWDatabase::InputId id = 0;
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
