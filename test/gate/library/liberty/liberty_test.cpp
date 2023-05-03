//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/library/liberty/translate.h"
#include "gate/library/liberty/net_data.h"
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
  const std::filesystem::path subCatalog = "test/data/gate/liberty";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::string filenames = prefixPath / "normal1.lib";
  translateLibertyToDesign(filenames, vec);
  return vec;
}

TEST(liberty_test, truthTableBuf) {
  auto bufGate = createLogicGate(GateSymbol::NOP);
  std::vector<uint64_t> expected = { 2 };
  EXPECT_EQ(NetData::buildTruthTab(bufGate.get()), expected);
}

TEST(liberty_test, Buf) {
  auto vec = libertyGnet();
  auto bufGate = createLogicGate(GateSymbol::NOP);
  EXPECT_EQ(NetData::buildTruthTab(bufGate.get()), NetData::buildTruthTab(vec.combNets[0].get()));
}

TEST(liberty_test, truthTableNot) {
  auto notGate = createLogicGate(GateSymbol::NOT);
  std::vector<uint64_t> expected = { 1 };
  EXPECT_EQ(NetData::buildTruthTab(notGate.get()), expected);
}

TEST(liberty_test, Not) {
  auto vec = libertyGnet();
  auto notGate = createLogicGate(GateSymbol::NOT);
  EXPECT_EQ(NetData::buildTruthTab(notGate.get()), NetData::buildTruthTab(vec.combNets[1].get()));
}

TEST(liberty_test, truthTableXor) {
  auto xorGate = createLogicGate(GateSymbol::XOR);
  std::vector<uint64_t> expected = { 6 };
  EXPECT_EQ(NetData::buildTruthTab(xorGate.get()), expected);
}

TEST(liberty_test, Xor) {
  auto vec = libertyGnet();
  auto xorGate = createLogicGate(GateSymbol::XOR);
  EXPECT_EQ(NetData::buildTruthTab(xorGate.get()), NetData::buildTruthTab(vec.combNets[2].get()));
}

TEST(liberty_test, truthTableOr) {
  auto orGate = createLogicGate(GateSymbol::OR);
  std::vector<uint64_t> expected = { 14 };
  EXPECT_EQ(NetData::buildTruthTab(orGate.get()), expected);
}

TEST(liberty_test, Or) {
  auto vec = libertyGnet();
  auto orGate = createLogicGate(GateSymbol::OR);
  EXPECT_EQ(NetData::buildTruthTab(orGate.get()), NetData::buildTruthTab(vec.combNets[3].get()));
}

TEST(liberty_test, truthTableAnd) {
  auto andGate = createLogicGate(GateSymbol::AND);
  std::vector<uint64_t> expected = { 8 };
  EXPECT_EQ(NetData::buildTruthTab(andGate.get()), expected);
}

TEST(liberty_test, And) {
  auto vec = libertyGnet();
  auto andGate = createLogicGate(GateSymbol::AND);
  EXPECT_EQ(NetData::buildTruthTab(andGate.get()), NetData::buildTruthTab(vec.combNets[4].get()));
}
