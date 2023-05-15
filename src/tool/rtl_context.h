//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//
#include "gate/debugger/base_checker.h"
#include "gate/debugger/checker.h"
#include "gate/model/gnet.h"
#include "gate/optimizer/optimizer.h"
#include "gate/optimizer/strategy/exhaustive_search_optimizer.h"
#include "gate/premapper/migmapper.h"
#include "gate/premapper/premapper.h"
#include "gate/premapper/xagmapper.h"
#include "gate/premapper/xmgmapper.h"
#include "gate/printer/graphml.h"
#include "options.h"
#include "rtl/compiler/compiler.h"
#include "rtl/library/arithmetic.h"
#include "rtl/library/flibrary.h"
#include "rtl/model/net.h"
#include "rtl/parser/ril/parser.h"

#include "easylogging++.h"

using VNet = eda::rtl::model::Net;
using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;
using Link = Gate::Link;

using AigMapper = eda::gate::premapper::AigMapper;
using Checker = eda::gate::debugger::Checker;
using Compiler = eda::rtl::compiler::Compiler;
using RewriteManager = eda::gate::optimizer::RewriteManager;
using LecType = eda::gate::debugger::options::LecType;
using Library = eda::rtl::library::ArithmeticLibrary;
using MigMapper = eda::gate::premapper::MigMapper;
using PreBasis = eda::gate::premapper::PreBasis;
using PreMapper = eda::gate::premapper::PreMapper;
using XagMapper = eda::gate::premapper::XagMapper;
using XmgMapper = eda::gate::premapper::XmgMapper;
using ESOptimizer = eda::gate::optimizer::ExhausitiveSearchOptimizer;

namespace eda::tool {

/**
 * \brief Implements a tool for processing RIL networks.
 */
struct RtlContext {
  RtlContext(const std::string &file):
    file(file) {}

  const std::string file;

  std::shared_ptr<VNet> vnet;
  std::shared_ptr<GNet> gnet0;
  std::shared_ptr<GNet> gnet1;
  std::shared_ptr<GNet> gnet2;
  std::shared_ptr<GNet> gnet3;

  PreMapper::GateIdMap gmap;

  bool equal;
  std::string techLib = "abc";
};

enum ParseResult {
  PARSE_INVALID,
  PARSE_RIL,
  PARSE_NETLIST
};

ParseResult parse(RtlContext &context);

bool compile(RtlContext &context);

bool premap(RtlContext &context, PreBasis basis);

bool optimize(RtlContext &context);

bool check(RtlContext &context, LecType type);

bool print(RtlContext &context, std::string file);

bool techMap(RtlContext &context);

std::string getName(std::string &path);

void fillingTechLib(std::string path);

int rtlMain(RtlContext &context, PreBasis basis, LecType type, 
  std::string file);

int rtlMain(RtlContext &context, const RtlOptions &options);

} // namespace eda::tool
