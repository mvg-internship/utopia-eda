//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//
#include "tool/rtl_context.h"
#include "gate/library/liberty/net_data.h"
#include "gate/library/liberty/translate.h"
#include "gate/model/utils.h"
#include "gate/optimizer/tech_map/strategy/replacement_cut.h"
#include "gate/optimizer/tech_map/strategy/simple_techmapper.h"
#include "gate/optimizer/tech_map/tech_mapper.h"
#include "gate/parser/bench/parser.h"
#include "gate/parser/glverilog/parser.h"

namespace eda::tool {

ParseResult parse(RtlContext &context) {
  LOG(INFO) << "RTL parse: " << context.file;

  std::vector<std::unique_ptr<GNet>> nets;
  if (parseGateLevelVerilog(context.file, nets)) {
    nets[0]->sortTopologically();
    context.gnet0 = std::shared_ptr<GNet>(nets[0].release());
    
    return PARSE_NETLIST;
  }

  try {
    context.gnet0 = parseBenchFile(context.file);
  } catch (std::exception& e) {
    std::cerr << "error in " << e.what() <<  std::endl; 
  }
  if (context.gnet0) {
    context.gnet0->sortTopologically();
    return PARSE_NETLIST;
  }

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

  eda::gate::optimizer::optimize(gnet2, 4, ESOptimizer("abc"));

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

bool techMap(RtlContext &context) {
  GNet *gnet3 = context.gnet2->clone();

  if (context.techLib != "abc") {
    eda::gate::optimizer::techMap(
        gnet3,
        4,
        eda::gate::optimizer::SimpleTechMapper(context.techLib.c_str()),
        eda::gate::optimizer::ReplacementVisitor());
  }

  context.gnet3 = std::shared_ptr<GNet>(gnet3);

  std::cout << "------ G-net #3 ------" << std::endl;
  dump(*context.gnet3);

  return true;
}

std::string getName(std::string &path) {
  return std::filesystem::path(path).filename();
}

void fillingTechLib(std::string path) {
  std::string namefile = getName(path);
  auto db = RewriteManager::get().createDatabase(namefile);
  NetData data;
  translateLibertyToDesign(path, data);
  data.fillDatabase(*db);
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
  if (!techMap(context)) { return -1; }
  if (!print(context, file)) { return -1; }

  return 0;
}

int rtlMain(RtlContext &context, const RtlOptions &options) {
  return rtlMain(context, options.preBasis, options.lecType,
   options.printGraphml);
}

} // namespace eda::tool
