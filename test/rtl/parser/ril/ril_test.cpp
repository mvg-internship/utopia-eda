//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "rtl/compiler/compiler.h"
#include "rtl/library/flibrary.h"
#include "rtl/model/net.h"
#include "rtl/parser/ril/builder.h"
#include "rtl/parser/ril/parser.h"

#include "gtest/gtest.h"

#include <iostream>
#include <memory>

using namespace eda::gate::model;
using namespace eda::rtl::compiler;
using namespace eda::rtl::library;
using namespace eda::rtl::model;
using namespace eda::rtl::parser::ril;

int rilTest(const std::string &filename) {
  auto model = parse(filename);

  std::cout << "------ p/v-nets ------" << std::endl;
  std::cout << *model << std::endl;

  Compiler compiler(FLibraryDefault::get());
  auto gnet = compiler.compile(*model);

  std::cout << "------ g-net ------" << std::endl;
  std::cout << *gnet;

  return 0;
}

TEST(RilTest, SingleTest) {
  EXPECT_EQ(rilTest("test/data/ril/test.ril"), 0);
}
