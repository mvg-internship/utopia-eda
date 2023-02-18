//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/rwdatabase/rwdatabase.h"

#include "gtest/gtest.h"

using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;

using RWDatabase = eda::gate::rwdatabase::RWDatabase;

bool insertTest() {
  RWDatabase rwdb;

  std::shared_ptr<GNet> dummy = std::make_shared<GNet>();
  RWDatabase::GateBinding binding = {{0, 1}, {1, 3}};
  RWDatabase::ValueVector valueVector = 1;
 
  rwdb.insert(valueVector, {{dummy, binding}});
  return ((rwdb.get(valueVector)[0].net == dummy) && (rwdb.get(valueVector)[0].binding == binding));
}

bool eraseTest() {
  RWDatabase rwdb;

  std::shared_ptr<GNet> dummy = std::make_shared<GNet>();
  RWDatabase::GateBinding binding = {{0, 1}, {1, 3}};
  RWDatabase::ValueVector valueVector = 1;

  bool flag1, flag2;
  rwdb.insert(valueVector, {{dummy, binding}});
  flag1 = !rwdb.empty();
  rwdb.erase(valueVector);
  flag2 = rwdb.empty();

  return flag1 && flag2;
}

TEST(RWDatabaseTest, InsertTest) {
  EXPECT_TRUE(insertTest());
}

TEST(RWDatabaseTest, EraseTest) {
  EXPECT_TRUE(eraseTest());
}
