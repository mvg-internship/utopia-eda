//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "tool/rtl_context.h"

using RtlContext = eda::tool::RtlContext;

// RIL tests for XAG-mapper
// Verify equivalence of premapped networks
// for arithmetic operations '+', '-', '*' described on RIL

TEST(XagPremapperRilTest, XagAddTest) {
  RtlContext context("../test/data/ril/add.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::DEFAULT, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagSubTest) {
  RtlContext context("../test/data/ril/sub.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::DEFAULT, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagMulTest) {
  RtlContext context("../test/data/ril/mul.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::DEFAULT, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagTestTest) {
  RtlContext context("data/ril/test.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::DEFAULT, "");
  EXPECT_TRUE(context.equal);
}

//
// BDDChecker
//

TEST(XagPremapperRilTest, XagAddBddTest) {
  RtlContext context("../test/data/ril/add.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::BDD, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagSubBddTest) {
  RtlContext context("../test/data/ril/sub.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::BDD, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagMulBddTest) {
  RtlContext context("../test/data/ril/mul.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::BDD, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagTestBddTest) {
  RtlContext context("data/ril/test.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::BDD, "");
  EXPECT_TRUE(context.equal);
}

//
// RandomChecker
//

TEST(XagPremapperRilTest, XagAddRndTest) {
  RtlContext context("../test/data/ril/add.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::RND, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagSubRndTest) {
  RtlContext context("../test/data/ril/sub.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::RND, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagMulRndTest) {
  RtlContext context("../test/data/ril/mul.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::RND, "");
  EXPECT_TRUE(context.equal);
}

TEST(XagPremapperRilTest, XagTestRndTest) {
  RtlContext context("data/ril/test.ril");
  eda::tool::rtlMain(context, PreBasis::XAG, LecType::RND, "");
  EXPECT_TRUE(context.equal);
}
