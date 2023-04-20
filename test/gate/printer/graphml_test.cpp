//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/printer/graphml.h"
#include "gate/model/gnet_test.h"

#include "gtest/gtest.h"

using namespace eda::printer::graphMl;

TEST(toGraphMlTest,all) {
  const GNet test = *makeRand(1000, 1000);
  toGraphMl::printer(std::cout, test);
}