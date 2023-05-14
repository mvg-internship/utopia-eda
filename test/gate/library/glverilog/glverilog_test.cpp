//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/library/liberty/net_data.h"
//#include "parser.h"
#include "gtest/gtest.h"
#include "gate/parser/glverilog/parse_fu.h"
#include <stdexcept>
#include <filesystem>

namespace GModel = eda::gate::model;

using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

static std::unique_ptr<GModel::GNet> createLogicGate(GateSymbol symbol) {
  std::unique_ptr<GModel::GNet> net = std::make_unique<GModel::GNet>();
  GateId a = net->addIn();
  GateId out;
  if (symbol != GateSymbol::DFF || symbol != GateSymbol::NOT) {
    GateId b = net->addIn();
    out = net->addGate(symbol, a, b);
  } else {
    out = net->addGate(symbol, a);
  }
  net->addOut(out);
  net->sortTopologically();
  return net;
}

inline static bool glverilogParser() {
  bool answer = true;
  std::vector<std::unique_ptr<GModel::GNet>> nets;
  const std::filesystem::path subCatalog = "test/data/glverilog/ISCAS";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::vector<std::string> files { "c17.v", "c432.v" };
  for (const auto &file : files) {
    const std::string filename = prefixPath / file;
    answer *= parseGateLevelVerilog(filename, nets);
  }
  return answer;
}

TEST(ISCAS, parse) {
  EXPECT_EQ(glverilogParser(), true);
}

TEST(gnetBuildTest, truthTableNot) {
  auto notGate = createLogicGate(GateSymbol::NOT);
  std::vector<uint64_t> expected = { 1 };
  EXPECT_EQ(NetData::buildTruthTab(notGate.get()), expected);
}

TEST(gnetBuildTest, truthTableOr) {
  auto orGate = createLogicGate(GateSymbol::OR);
  std::vector<uint64_t> expected = { 14 };
  EXPECT_EQ(NetData::buildTruthTab(orGate.get()), expected);
}

TEST(gnetBuildTest, truthTableXor) {
  auto xorGate = createLogicGate(GateSymbol::XOR);
  std::vector<uint64_t> expected = { 6 };
  EXPECT_EQ(NetData::buildTruthTab(xorGate.get()), expected);
}

TEST(gnetBuildTest, truthTableAnd) {
  auto andGate = createLogicGate(GateSymbol::AND);
  std::vector<uint64_t> expected = { 8 };
  EXPECT_EQ(NetData::buildTruthTab(andGate.get()), expected);
}