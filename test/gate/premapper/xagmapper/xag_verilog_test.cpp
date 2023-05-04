//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <lorina/diagnostics.hpp>
#include <lorina/verilog.hpp>

#include "gate/debugger/checker.h"
#include "gate/parser/gate_verilog_parser.h"
#include "gate/printer/dot.h"
#include "gate/premapper/xagmapper/xag_test.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <unordered_map>

using namespace eda::gate::premapper;
using namespace eda::gate::model;
using namespace lorina;

using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
using GateIdMap = std::unordered_map<Gate::Id, Gate::Id>;
using Link = Gate::Link;

const std::filesystem::path subCatalog = "test/data/gate/premapper/xagmapper";
const std::filesystem::path homePath = std::string(getenv("UTOPIA_HOME"));
const std::filesystem::path prefixPath = homePath / subCatalog;
const std::filesystem::path prefixPathIn = prefixPath;

bool parseFile(const std::string g) {
  std::string infile = g;
  std::string filename = prefixPathIn / (infile + ".v");
  text_diagnostics consumer;
  diagnostic_engine diag(&consumer);
  GateVerilogParser parser(infile);
  return_code result = read_verilog(filename, parser, &diag);
  EXPECT_EQ(result, return_code::success);

  std::shared_ptr<GNet> net = std::make_shared<GNet>(*parser.getGnet());
  net->sortTopologically();
  GateIdMap gmap;

  std::shared_ptr<GNet> premapped = premap(net, gmap);
  delete parser.getGnet();
  return checkEquivalence(net, premapped, gmap);
}

TEST(XagPremapperVerilogTest, orGateTest) {
  parseFile("orGate");
}

TEST(XagPremapperVerilogTest, xorGateTest) {
  parseFile("xorGate");
}

TEST(XagPremapperVerilogTest, xnorGateTest) {
  parseFile("xnorGate");
}

TEST(XagPremapperVerilogTest, norGateTest) {
  parseFile("norGate");
}

TEST(XagPremapperVerilogTest, nandGateTest) {
  parseFile("nandGate");
}

TEST(XagPremapperVerilogTest, MultiplexerTest) {
  parseFile("multiplexer");
}

TEST(XagPremapperVerilogTest, halfSubtractorTest) {
  parseFile("halfSubtractor");
}
