//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//
#include "tool/rtl_context.h"
#include "gate/model/utils.h"

namespace eda::tool {

ParseResult parse(RtlContext &context) {
  LOG(INFO) << "RTL parse: " << context.file;

  context.vnet = eda::rtl::parser::ril::parse(context.file);

  if (context.vnet) {
    std::cout << "------ P/V-nets ------" << std::endl;
    std::cout << *context.vnet << std::endl;

    return PARSE_RIL;
  }

  LOG(ERROR) << "Could not parse the file";
  return PARSE_INVALID;
}

bool compile(RtlContext &context) {
  LOG(INFO) << "RTL compile";

  Compiler compiler(Library::get());
  context.gnet0 = compiler.compile(*context.vnet);

  if (context.gnet0 == nullptr) {
    LOG(ERROR) << "Could not compile the model";
    return false;
  }

  context.gnet0->sortTopologically();

  std::cout << "------ G-net #0 ------" << std::endl;
  eda::gate::model::dump(*context.gnet0);

  return true;
}

bool premap(RtlContext &context, PreBasis basis) {
  LOG(INFO) << "RTL premap";

  auto &premapper =
      eda::gate::premapper::getPreMapper(basis);

  context.gnet1 = premapper.map(*context.gnet0, context.gmap);
  context.gnet1->sortTopologically();

  std::cout << "------ G-net #1 ------" << std::endl;
  eda::gate::model::dump(*context.gnet1);

  return true;
}

bool optimize(RtlContext &context) {
  GNet *gnet2 = context.gnet1->clone();

  eda::gate::optimizer::optimize(gnet2, 4, ESOptimizer());

  context.gnet2 = std::shared_ptr<GNet>(gnet2);

  std::cout << "------ G-net #2 ------" << std::endl;
  eda::gate::model::dump(*context.gnet2);

  return true;
}

bool check(RtlContext &context, LecType type) {
  LOG(INFO) << "RTL check";

  auto &checker = eda::gate::debugger::getChecker(type);

  assert(context.gnet0->nSourceLinks() == context.gnet1->nSourceLinks());
  assert(context.gnet0->nTargetLinks() == context.gnet1->nTargetLinks());

  context.equal = checker.areEqual(*context.gnet0, *context.gnet1, context.gmap);
  std::cout << "equivalent=" << context.equal << std::endl;

  return true;
}

bool print(RtlContext &context, std::string file) {
  std::ofstream fout;
  fout.open(file);
  eda::printer::graphMl::toGraphMl::printer(fout, *context.gnet1);
  fout.close();
  return true;
}

int rtlMain(
    RtlContext &context, PreBasis basis, LecType type, std::string file) {
  ParseResult rc = parse(context);
  if (rc == PARSE_INVALID) {
    return -1;
  }
  if (rc == PARSE_RIL) {
    if (!compile(context)) {
      return -1;
    }
  }
  if (!premap(context, basis))  { return -1; }
  if (!optimize(context)) { return -1; }
  if (!check(context, type))   { return -1; }
  if (!print(context, file)) { return -1; }

  return 0;
}

int rtlMain(RtlContext &context, const RtlOptions &options) {
  return rtlMain(context, options.preBasis, options.lecType,
   options.printGraphml);
}

} // namespace eda::tool
