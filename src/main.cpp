//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//
#include "config.h"
#include "gate/debugger/base_checker.h"
#include "gate/debugger/checker.h"
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/premapper/migmapper.h"
#include "gate/premapper/premapper.h"
#include "gate/premapper/xagmapper.h"
#include "gate/premapper/xmgmapper.h"
#include "options.h"
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

INITIALIZE_EASYLOGGINGPP

//===-----------------------------------------------------------------------===/
// Logic Synthesis
//===-----------------------------------------------------------------------===/

struct RtlContext {
  using VNet = eda::rtl::model::Net;
  using GNet = eda::gate::model::GNet;
  using Gate = eda::gate::model::Gate;
  using Link = Gate::Link;

  using AigMapper = eda::gate::premapper::AigMapper;
  using Checker = eda::gate::debugger::Checker;
  using Compiler = eda::rtl::compiler::Compiler;
  using Library = eda::rtl::library::FLibraryDefault;
  using MigMapper = eda::gate::premapper::MigMapper;
  using PreBasis = eda::gate::premapper::PreBasis;
  using PreMapper = eda::gate::premapper::PreMapper;
  using XagMapper = eda::gate::premapper::XagMapper;
  using XmgMapper = eda::gate::premapper::XmgMapper;

  RtlContext(const std::string &file, const RtlOptions &options):
    file(file), options(options) {}

  const std::string file;
  const RtlOptions &options;

  std::shared_ptr<VNet> vnet;
  std::shared_ptr<GNet> gnet0;
  std::shared_ptr<GNet> gnet1;

  PreMapper::GateIdMap gmap;

  bool equal;
};

void dump(const GNet &net) {
  std::cout << net << std::endl;

  for (auto source : net.sourceLinks()) {
    const auto *gate = RtlContext::Gate::get(source.target);
    std::cout << *gate << std::endl;
  }
  for (auto target : net.targetLinks()) {
    const auto *gate = RtlContext::Gate::get(target.source);
    std::cout << *gate << std::endl;
  }

  std::cout << std::endl;
  std::cout << "N=" << net.nGates() << std::endl;
  std::cout << "I=" << net.nSourceLinks() << std::endl;
  std::cout << "O=" << net.nTargetLinks() << std::endl;
}

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
      eda::gate::premapper::getPreMapper(context.options.preBasis);

  context.gnet1 = premapper.map(*context.gnet0, context.gmap);

  std::cout << "------ G-net #1 ------" << std::endl;
  dump(*context.gnet1);

  return true;
}

bool check(RtlContext &context) {
  LOG(INFO) << "RTL check";

  auto &checker = eda::gate::debugger::getChecker(context.options.lecType);

  assert(context.gnet0->nSourceLinks() == context.gnet1->nSourceLinks());
  assert(context.gnet0->nTargetLinks() == context.gnet1->nTargetLinks());

  context.equal = checker.areEqual(*context.gnet0, *context.gnet1, context.gmap);
  std::cout << "equivalent=" << context.equal << std::endl;

  return true;
}

int rtlMain(RtlContext &context) {
  if (!parse(context))   { return -1; }
  if (!compile(context)) { return -1; }
  if (!premap(context))  { return -1; }
  if (!check(context))   { return -1; }

  return 0;
}

int main(int argc, char **argv) {
  START_EASYLOGGINGPP(argc, argv);

  std::stringstream title;
  std::stringstream version;

  version << VERSION_MAJOR << "." << VERSION_MINOR;

  title << "Utopia EDA " << version.str() << " | ";
  title << "Copyright (c) 2021-2022 ISPRAS";

  Options options(title.str(), version.str());

  try {
    options.initialize("config.json", argc, argv);
  } catch(const CLI::ParseError &e) {
    return options.exit(e);
  }

  int result = 0;

  for (auto file : options.rtl.files()) {
    RtlContext context(file, options.rtl);
    result |= rtlMain(context);
  }

  return result;
}