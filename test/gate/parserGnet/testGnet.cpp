#include "gate/parserGnet/parserGnet.h"
#include "gtest/gtest.h"

#include <iostream>

namespace GModel = eda::gate::model;

using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

static std::unique_ptr<GModel::GNet> createLogicGate2(
    GateSymbol symbol) {
  std::unique_ptr<GModel::GNet> net = std::make_unique<GModel::GNet>();
  GateId a = net->addIn();
  GateId b = net->addIn();
  GateId out = net->addGate(symbol, a, b);
  net->addOut(out);
  net->sortTopologically();
  return net;
}

static std::unique_ptr<GModel::GNet> createLogicGate1(
    GateSymbol symbol) {
  std::unique_ptr<GModel::GNet> net = std::make_unique<GModel::GNet>();
  GateId a = net->addIn();
  GateId out = net->addGate(symbol, a);
  net->addOut(out);
  net->sortTopologically();
  return net;
}

static void fillSource(NetData &source) {
  auto bufGate = createLogicGate1(GateSymbol::NOP);
  auto notGate = createLogicGate1(GateSymbol::NOT);
  auto xorGate = createLogicGate2(GateSymbol::XOR);
  auto orGate = createLogicGate2(GateSymbol::OR);
  auto andGate = createLogicGate2(GateSymbol::AND);
  source.combNets.push_back(*bufGate);
  source.combNets.push_back(*notGate);
  source.combNets.push_back(*xorGate);
  source.combNets.push_back(*orGate);
  source.combNets.push_back(*andGate);
}

static std::vector<uint64_t> fillTable(std::unique_ptr<NetData> vec) {
  std::vector<uint64_t> answ;
  for (auto it: vec->combNets) {
    auto it1 = std::make_unique<const eda::gate::model::GNet>(it);
    auto table = truthTab(std::move(it1));
    answ.insert(answ.end(), table.begin(), table.end());
  }
  return answ;
}

static bool equalTable() {
  std::unique_ptr<NetData> vec = std::make_unique<NetData>();
  std::unique_ptr<NetData> source = std::make_unique<NetData>();
  fillSource(*source);
  translateLibertyToDesign("normal1.lib", *vec);
  return fillTable(std::move(vec)) == fillTable(std::move(source));
}

TEST(testGnet, equalTable) {
  EXPECT_TRUE(equalTable());
}

