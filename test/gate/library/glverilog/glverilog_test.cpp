//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/library/liberty/net_data.h"
#include "gtest/gtest.h"
#include "gate/parser/glverilog/parser.h"
#include <stdexcept>
#include <filesystem>

namespace GModel = eda::gate::model;

using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

static std::vector<std::unique_ptr<GModel::GNet>> createLogicGate(const std::string file) {
 std::unique_ptr<GModel::GNet> net = std::make_unique<GModel::GNet>();
 std::vector<std::unique_ptr<GModel::GNet>> nets;
  const std::filesystem::path subCatalog = "test/data/glverilog";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::string filename = prefixPath / file;
  parseGateLevelVerilog(filename,nets);
  
  
  return nets;
}

inline static bool glverilogParser() {
  bool answer = true;
  std::vector<std::unique_ptr<GModel::GNet>> nets;
  const std::filesystem::path subCatalog = "test/data/glverilog";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::vector<std::string> files { "ISCAS/c17.v", "ISCAS/c432.v" };
  for (const auto &file : files) {
    const std::string filename = prefixPath / file;
    answer *= parseGateLevelVerilog(filename, nets);
  }
  return answer;
}

TEST(glVerilogTranslator, ISCAS) {
  EXPECT_EQ(glverilogParser(), true);
}

TEST(glVerilogTranslator, parseNot) {
  auto net = std::move(createLogicGate("logic_gates/not.v").front());
  net->sortTopologically();
  std::vector<uint64_t> expected = { 1 };
  EXPECT_EQ(NetData::buildTruthTab(net.get()), expected);
}

TEST(glVerilogTranslator, parseNor) {
  auto net = std::move(createLogicGate("logic_gates/nor.v").front());
  net->sortTopologically();
  std::vector<uint64_t> expected = { 1 };
  EXPECT_EQ(NetData::buildTruthTab(net.get()), expected);
}

TEST(glVerilogTranslator, parseNand) {
  auto net = std::move(createLogicGate("logic_gates/nand.v").front());
  net->sortTopologically();
  std::vector<uint64_t> expected = { 7 };
  EXPECT_EQ(NetData::buildTruthTab(net.get()), expected);
}

TEST(glVerilogTranslator, parseXnor) {
  auto net = std::move(createLogicGate("logic_gates/xnor.v").front());
  net->sortTopologically();
  std::vector<uint64_t> expected = { 9 };
  EXPECT_EQ(NetData::buildTruthTab(net.get()), expected);
}

TEST(glVerilogTranslator, parseOr) {
  auto net = std::move(createLogicGate("logic_gates/or.v").front());
  net->sortTopologically();
  std::vector<uint64_t> expected = { 14 };
  EXPECT_EQ(NetData::buildTruthTab(net.get()), expected);
}

TEST(glVerilogTranslator, parseXor) {
  auto net = std::move(createLogicGate("logic_gates/xor.v").front());
  net->sortTopologically();
  std::vector<uint64_t> expected = { 6 };
  EXPECT_EQ(NetData::buildTruthTab(net.get()), expected);
}

TEST(glVerilogTranslator, parseAnd) {
  auto net = std::move(createLogicGate("logic_gates/and.v").front());
  net->sortTopologically();
  std::vector<uint64_t> expected = { 8 };
  EXPECT_EQ(NetData::buildTruthTab(net.get()), expected);
}
