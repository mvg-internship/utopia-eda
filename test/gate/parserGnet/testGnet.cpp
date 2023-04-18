#include "gate/parserGnet/parserGnet.h"
#include "gtest/gtest.h"

namespace GModel = eda::gate::model;

using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

static std::unique_ptr<GModel::GNet> createLogicGate1(
    GateSymbol symbol) {
  std::unique_ptr<GModel::GNet> net = std::make_unique<GModel::GNet>();
  GateId a = net->addIn();
  GateId out = net->addGate(symbol, a);
  net->addOut(out);
  net->sortTopologically();
  return net;
}

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

static void fillSource(NetData &source) {
  auto bufGate = createLogicGate1(GateSymbol::NOP);
  auto notGate = createLogicGate1(GateSymbol::NOT);
  auto xorGate = createLogicGate2(GateSymbol::XOR);
  auto orGate = createLogicGate2(GateSymbol::OR);
  auto andGate = createLogicGate2(GateSymbol::AND);
  source.combNets.push_back(std::move(bufGate));
  source.combNets.push_back(std::move(notGate));
  source.combNets.push_back(std::move(xorGate));
  source.combNets.push_back(std::move(orGate));
  source.combNets.push_back(std::move(andGate));
}

std::vector<uint64_t> fillTable(const NetData& vec) {
    std::vector<uint64_t> table;
    for (const auto &it: vec.combNets) {
        auto result = truthTab(it.get());
        table.insert(table.end(), result.begin(), result.end());
    }
    return table;
}

static bool equalTable() {
  NetData vec, source;
  fillSource(source);
  translateLibertyToDesign("normal1.lib", vec);
  return fillTable(vec) == fillTable(source);
}

TEST(testGnet, equalTable) {
  EXPECT_TRUE(equalTable());
}

