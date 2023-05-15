//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/parser/bench/parser.h"
#include "gate/library/liberty/net_data.h"
#include "gtest/gtest.h"

#include <filesystem>

namespace GModel = eda::gate::model;

using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

static std::unique_ptr<GModel::GNet> buildGate(const std::string file) {
  std::unique_ptr<GModel::GNet> net = std::make_unique<GModel::GNet>();
  const std::filesystem::path subCatalog = "test/data/bench";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::string filename = prefixPath / file;
  net = parseBenchFile(filename);
  net->sortTopologically();
  return net;
}

inline static bool benchParser() {
  bool answer = true;
  const std::filesystem::path subCatalog = "test/data/bench";
  const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
  const std::filesystem::path prefixPath = homePath / subCatalog;
  const std::vector<std::string> files { "s27.bench", "s298.bench" };
  for (const auto &file : files) {
    const std::string filename = prefixPath / file;
    answer *= (parseBenchFile(filename) == nullptr ? false : true);
  }
  return answer;
}

TEST(ISCAS, parse) {
  EXPECT_EQ(benchParser(), true);
}

TEST(benchTranslator, parseNot) {
  auto notGate = buildGate("not.bench");
  std::vector<uint64_t> expected = { 1 };
  EXPECT_EQ(NetData::buildTruthTab(notGate.get()), expected);
}

TEST(benchTranslator, parseOr) {
  auto orGate = buildGate("or.bench");
  std::vector<uint64_t> expected = { 14 };
  EXPECT_EQ(NetData::buildTruthTab(orGate.get()), expected);
}

TEST(benchTranslator, parseAnd) {
  auto andGate = buildGate("and.bench");
  std::vector<uint64_t> expected = { 8 };
  EXPECT_EQ(NetData::buildTruthTab(andGate.get()), expected);
}

TEST(benchTranslator, parseNand) {
  auto nandGate = buildGate("nand.bench");
  std::vector<uint64_t> expected = { 7 };
  EXPECT_EQ(NetData::buildTruthTab(nandGate.get()), expected);
}

TEST(benchTranslator, parseNor) {
  auto norGate = buildGate("nor.bench");
  std::vector<uint64_t> expected = { 1 };
  EXPECT_EQ(NetData::buildTruthTab(norGate.get()), expected);
}
