//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//
#include "config.h"
#include "gate/model/gate.h"
#include "tool/rtl_context.h"
#include "util/string.h"

#include "easylogging++.h"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

using RtlContext = eda::tool::RtlContext;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv) {
  START_EASYLOGGINGPP(argc, argv);

  std::stringstream title;
  std::stringstream version;

  version << VERSION_MAJOR << "." << VERSION_MINOR;

  title << "Utopia EDA " << version.str() << " | ";
  title << "Copyright (c) 2021-2023 ISPRAS";

  Options options(title.str(), version.str());

  try {
    options.initialize("config.json", argc, argv);
  } catch (const CLI::ParseError &e) {
    return options.exit(e);
  }

  int result = 0;
  std::string nameFileLibrary;

  if (!options.rtl.libertyFile.empty()) {
    eda::tool::fillingTechLib(options.rtl.libertyFile);
    nameFileLibrary = eda::tool::getName(options.rtl.libertyFile);
  }
  for (auto file : options.rtl.files()) {
    RtlContext context(file);
    if (!nameFileLibrary.empty()) {
      context.techLib = nameFileLibrary;
    }
    result |= eda::tool::rtlMain(context, options.rtl);
  }

  return result;
}
