//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "config.h"
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
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

  eda::rtl::compiler::Compiler compiler(
      eda::rtl::library::FLibraryDefault::get());
  auto netlist = compiler.compile(*model);

  std::cout << "------ netlist ------" << std::endl;
  std::cout << *netlist;

  std::cout << "Netlist: nGates=" << netlist->nGates() << std::endl;

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
