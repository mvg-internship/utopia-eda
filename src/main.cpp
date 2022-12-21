//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "config.h"
#include "gate/debugger/checker.h"
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/premapper/aigmapper.h"
#include "options.h"
#include "rtl/compiler/compiler.h"
#include "rtl/library/flibrary.h"
#include "rtl/model/net.h"
#include "rtl/parser/ril/parser.h"
#include "util/string.h"

#include "easylogging++.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

INITIALIZE_EASYLOGGINGPP

int rtlMain(const std::string &file, const RtlOptions &options) {
  LOG(INFO) << "Starting rtlMain " << file;

  auto model = eda::rtl::parser::ril::parse(file);
  if (model == nullptr) {
    std::cout << "Could not parse " << file << std::endl;
    std::cout << "Synthesis terminated." << std::endl;
    return -1;
  }

  std::cout << "------ p/v-nets ------" << std::endl;
  std::cout << *model << std::endl;

  //===---------------------------------------------------------------------===/
  // Netlist compilation
  //===---------------------------------------------------------------------===/
  using Compiler = eda::rtl::compiler::Compiler;
  using FLibraryDefault = eda::rtl::library::FLibraryDefault;

  Compiler compiler(FLibraryDefault::get());
  auto net = compiler.compile(*model);

  net->sortTopologically();

  std::cout << "------ netlist (original) ------" << std::endl;
  std::cout << *net;

  std::cout << "Net: nGates=" << net->nGates() << std::endl;

  //===---------------------------------------------------------------------===/
  // Pre-mapping
  //===---------------------------------------------------------------------===/
  using AigMapper = eda::gate::premapper::AigMapper;
  using GateIdMap = AigMapper::GateIdMap;

  GateIdMap oldToNewGates;
  auto &premapper = AigMapper::get();
  auto premapped = premapper.map(*net, oldToNewGates);

  std::cout << "------ netlist (premapped) ------" << std::endl;
  std::cout << *premapped;

  std::cout << "Net: nGates=" << premapped->nGates() << std::endl;

  //===---------------------------------------------------------------------===/
  // Equivalence checking
  //===---------------------------------------------------------------------===/
  using Link = eda::gate::model::Gate::Link;
  using Checker = eda::gate::debugger::Checker;
  using GateBinding = Checker::GateBinding;
  using Hints = Checker::Hints;

  Checker checker;
  GateBinding imap, omap, tmap;

  for (auto oldSourceLink : net->sourceLinks()) {
    auto newSourceId = oldToNewGates[oldSourceLink.target];
    imap.insert({oldSourceLink, Link(newSourceId)});
  }

  for (auto oldTriggerId : net->triggers()) {
    auto newTriggerId = oldToNewGates[oldTriggerId];
    tmap.insert({Link(oldTriggerId), Link(newTriggerId)});
  }

  // TODO: Here are only triggers.
  for (auto oldTriggerId : net->triggers()) {
    auto newTriggerId = oldToNewGates[oldTriggerId];
    auto *oldTrigger = Gate::get(oldTriggerId);
    auto *newTrigger = Gate::get(newTriggerId);

    auto oldDataId = oldTrigger->input(0).node();
    auto newDataId = newTrigger->input(0).node();

    omap.insert({Link(oldDataId), Link(newDataId)});
  }

  Hints hints;
  hints.sourceBinding = std::make_shared<GateBinding>(std::move(imap));
  hints.targetBinding = std::make_shared<GateBinding>(std::move(omap));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tmap));

  std::cout << "Equivalent: " << checker.areEqual(*net, *premapped, hints); 

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
    result |= rtlMain(file, options.rtl);
  }

  return result;
}
