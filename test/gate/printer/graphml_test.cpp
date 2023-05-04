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

using GNet = eda::gate::model::GNet;

namespace eda::printer::graphMl {

int graphMlTest() {
  const GNet test = *eda::gate::model::makeRand(1000, 1000);
  toGraphMl::printer(std::cout, test);
  return 0;
}

TEST(toGraphMlTest,all) {
  EXPECT_EQ(graphMlTest(), 0);
}

} // namespace eda::printer::graphMl