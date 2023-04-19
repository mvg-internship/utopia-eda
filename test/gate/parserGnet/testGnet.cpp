#include "gate/parserGnet/parserGnet.h"
#include "gtest/gtest.h"

#include <filesystem>

namespace GModel = eda::gate::model;

using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

static std::unique_ptr<GModel::GNet> createLogicGate(
    GateSymbol symbol) {
  std::unique_ptr<GModel::GNet> net = std::make_unique<GModel::GNet>();
  GateId a = net->addIn();
  GateId out;
  if (symbol != GateSymbol::NOP && symbol != GateSymbol::NOT) {
    GateId b = net->addIn();
    out = net->addGate(symbol, a, b);
  } else {
    out = net->addGate(symbol, a);
  }
  net->addOut(out);
  net->sortTopologically();
  return net;
}

inline static NetData libertyGnet() {
  NetData vec;
  const std::filesystem::path subCatalog = "test/data/gate/parserGnet";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::string filenames = prefixPath / "normal1.lib";
  translateLibertyToDesign(filenames, vec);
  return vec;
}

TEST(testGnet, Buf) {
  auto vec = libertyGnet();
  auto bufGate = createLogicGate(GateSymbol::NOP);
  EXPECT_EQ(buildTruthTab(bufGate.get()), buildTruthTab(vec.combNets[0].get()));
}

TEST(testGnet, Not) {
  auto vec = libertyGnet();
  auto notGate = createLogicGate(GateSymbol::NOT);
  EXPECT_EQ(buildTruthTab(notGate.get()), buildTruthTab(vec.combNets[1].get()));
}

TEST(testGnet, Xor) {
  auto vec = libertyGnet();
  auto xorGate = createLogicGate(GateSymbol::XOR);
  EXPECT_EQ(buildTruthTab(xorGate.get()), buildTruthTab(vec.combNets[2].get()));
}

TEST(testGnet, Or) {
  auto vec = libertyGnet();
  auto orGate = createLogicGate(GateSymbol::OR);
  EXPECT_EQ(buildTruthTab(orGate.get()), buildTruthTab(vec.combNets[3].get()));
}

TEST(testGnet, And) {
  auto vec = libertyGnet();
  auto andGate = createLogicGate(GateSymbol::AND);
  EXPECT_EQ(buildTruthTab(andGate.get()), buildTruthTab(vec.combNets[4].get()));
}
