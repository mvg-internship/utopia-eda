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

#include "easylogging++.h"

using VNet = eda::rtl::model::Net;
using GNet = eda::gate::model::GNet;
using Gate = eda::gate::model::Gate;
using Link = Gate::Link;

using AigMapper = eda::gate::premapper::AigMapper;
using Checker = eda::gate::debugger::Checker;
using Compiler = eda::rtl::compiler::Compiler;
using LecType = eda::gate::debugger::options::LecType;
using Library = eda::rtl::library::FLibraryDefault;
using MigMapper = eda::gate::premapper::MigMapper;
using PreBasis = eda::gate::premapper::PreBasis;
using PreMapper = eda::gate::premapper::PreMapper;
using XagMapper = eda::gate::premapper::XagMapper;
using XmgMapper = eda::gate::premapper::XmgMapper;

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

  PreMapper::GateIdMap gmap;

  bool equal;
};

bool parse(RtlContext &context);

bool compile(RtlContext &context);

bool premap(RtlContext &context, PreBasis basis);

bool check(RtlContext &context, LecType type);

int rtlMain(RtlContext &context, PreBasis basis, LecType type);
int rtlMain(RtlContext &context, const RtlOptions &options);

} // namespace eda::tool