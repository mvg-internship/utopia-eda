//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "gate/debugger/base_checker.h"
#include "gate/debugger/checker.h"
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/model/gnet_test.h"
#include "gate/premapper/migmapper.h"
#include "gate/premapper/premapper.h"
#include "gate/premapper/xagmapper.h"
#include "gate/premapper/xmgmapper.h"
#include "rtl/compiler/compiler.h"
#include "rtl/library/arithmetic.h"
#include "rtl/library/flibrary.h"
#include "rtl/model/net.h"
#include "rtl/parser/ril/parser.h"
#include "util/string.h"

#include "easylogging++.h"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

// RIL tests for XAG-mapper
// Verify equivalence of premapped networks for arithmetic operations '+', '-', '*' described on RIL

struct RtlContext {
  using VNet = eda::rtl::model::Net;
  using GNet = eda::gate::model::GNet;
  using Gate = eda::gate::model::Gate;
  using Link = Gate::Link;
  using Checker = eda::gate::debugger::Checker;
  using Compiler = eda::rtl::compiler::Compiler;
  using Library = eda::rtl::library::FLibraryDefault;
  using PreBasis = eda::gate::premapper::PreBasis;
  using PreMapper = eda::gate::premapper::PreMapper;
  using XagMapper = eda::gate::premapper::XagMapper;

  RtlContext(const std::string &file):
    file(file) {}

  const std::string file;

  std::shared_ptr<VNet> vnet;
  std::shared_ptr<GNet> gnet0;
  std::shared_ptr<GNet> gnet1;

  PreMapper::GateIdMap gmap;

  bool equal;
};

bool parse(RtlContext &context) {
  LOG(INFO) << "RTL parse: " << context.file;

  context.vnet = eda::rtl::parser::ril::parse(context.file);

  if (context.vnet == nullptr) {
    LOG(ERROR) << "Could not parse the file";;
    return false;
  }

  std::cout << "------ P/V-nets ------" << std::endl;
  std::cout << *context.vnet << std::endl;

  return true;
}

bool compile(RtlContext &context) {
  LOG(INFO) << "RTL compile";

  RtlContext::Compiler compiler(RtlContext::Library::get());
  context.gnet0 = compiler.compile(*context.vnet);

  if (context.gnet0 == nullptr) {
    LOG(ERROR) << "Could not compile the model";
    return false;
  }

  context.gnet0->sortTopologically();

  std::cout << "------ G-net #0 ------" << std::endl;
  dump(*context.gnet0);

  return true;
}

bool premap(RtlContext &context) {
  LOG(INFO) << "RTL premap";

  auto &premapper =
      eda::gate::premapper::getPreMapper(eda::gate::premapper::PreBasis::XAG);

  context.gnet1 = premapper.map(*context.gnet0, context.gmap);
  context.gnet1->sortTopologically();

  std::cout << "------ G-net #1 ------" << std::endl;
  eda::gate::model::dump(*context.gnet1);

  return true;
}

bool check(RtlContext &context) {
  LOG(INFO) << "RTL check";

  auto &checker = eda::gate::debugger::getChecker(eda::gate::debugger::options::LecType::DEFAULT);

  assert(context.gnet0->nSourceLinks() == context.gnet1->nSourceLinks());
  assert(context.gnet0->nTargetLinks() == context.gnet1->nTargetLinks());

  context.equal = checker.areEqual(*context.gnet0, *context.gnet1, context.gmap);
  std::cout << "equivalent=" << context.equal << std::endl;

  return context.equal;
}

int rtlMain(RtlContext &context) {
  if (!parse(context))   { return -1; }
  if (!compile(context)) { return -1; }
  if (!premap(context))  { return -1; }

  return check(context);
}

TEST(XagPremapperRilTest, XagAddTest) {
  RtlContext context("data/ril/add.ril");
  EXPECT_TRUE(rtlMain(context));
}

TEST(XagPremapperRilTest, XagSubTest) {
  RtlContext context("data/ril/sub.ril");
  EXPECT_TRUE(rtlMain(context));
}

TEST(XagPremapperRilTest, XagMulTest) {
  RtlContext context("data/ril/mul.ril");
  EXPECT_TRUE(rtlMain(context));
}

// TODO: BDD have no dff trigger gate support!

//TEST(XagPremapperRilTest, XagTestTest) {
//  RtlContext context("data/ril/test.ril");
//  EXPECT_TRUE(rtlMain(context));
//}

