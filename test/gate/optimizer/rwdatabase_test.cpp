//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/transformer/bdd.h"
#include "gate/optimizer/rwdatabase.h"

#include "gtest/gtest.h"

using BDDList = eda::gate::transformer::GNetBDDConverter::BDDList;
using Gate = eda::gate::model::Gate;
using GateBDDMap = eda::gate::transformer::GNetBDDConverter::GateBDDMap;
using GateList = std::vector<Gate::Id>;
using GateSymbol = eda::gate::model::GateSymbol;
using GateUintMap = eda::gate::transformer::GNetBDDConverter::GateUintMap;
using GNet = eda::gate::model::GNet;
using GNetBDDConverter = eda::gate::transformer::GNetBDDConverter;
using RWDatabase = eda::gate::optimizer::RWDatabase;
using SQLiteRWDatabase = eda::gate::optimizer::SQLiteRWDatabase;

static std::unique_ptr<GNet> makeAnd2(Gate::SignalList
                                      &inputs,
                                      Gate::Id &outputId,
                                      GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0Id = net->newGate(), x1Id = net->newGate();
  Gate::SignalList and0Inputs = {Gate::Signal::always(x0Id),
                                 Gate::Signal::always(x1Id)};

  inputs = and0Inputs;
  Gate::Id andOutputId = net->addGate(GateSymbol::AND, and0Inputs);
  outputId = net->addGate(GateSymbol::OUT,
                          {Gate::Signal::always(andOutputId)});
  varList = {x0Id, x1Id};

  net->sortTopologically();

  return net;
}

static std::unique_ptr<GNet> makeOr2(Gate::SignalList &inputs,
                                     Gate::Id &outputId,
                                     GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0Id = net->newGate(), x1Id = net->newGate();
  Gate::SignalList or0Inputs = {Gate::Signal::always(x0Id),
                                Gate::Signal::always(x1Id)};

  inputs = or0Inputs;
  Gate::Id orOutputId = net->addGate(GateSymbol::OR, or0Inputs);
  outputId = net->addGate(GateSymbol::OUT,
                          {Gate::Signal::always(orOutputId)});
  varList = {x0Id, x1Id};

  net->sortTopologically();

  return net;
}

bool areEquivalent(RWDatabase::BoundGNet bgnet1, RWDatabase::BoundGNet bgnet2) {
  Cudd manager(0, 0);
  BDDList x = { manager.bddVar(), manager.bddVar() };
  GateBDDMap varMap1, varMap2;

  for (auto p : bgnet1.bindings) {
    varMap1[p.second] = x[p.first];
  }
  for (auto p : bgnet2.bindings) {
    varMap2[p.second] = x[p.first];
  }

  BDD bdd1 = GNetBDDConverter::convert(*bgnet1.net,
                                       bgnet1.net->
                                       targetLinks().begin()->target,
                                       varMap1, manager);
  BDD bdd2 = GNetBDDConverter::convert(*bgnet2.net,
                                       bgnet2.net->
                                       targetLinks().begin()->target,
                                       varMap2, manager);

  return bdd1 == bdd2;
}

bool basicTest() {
  RWDatabase rwdb;
  bool result = true;

  std::shared_ptr<GNet> dummy = std::make_shared<GNet>();
  RWDatabase::GateBindings bindings = {{0, 1}, {1, 3}};
  RWDatabase::TruthTable truthTable = 8;

  rwdb.set(truthTable, {{dummy, bindings}});
  result = result && ((rwdb.get(truthTable)[0].net == dummy) &&
           (rwdb.get(truthTable)[0].bindings == bindings));

  result = result && !rwdb.empty();
  rwdb.erase(truthTable);
  result = result && rwdb.empty();

  return result;
}

bool insertGetARWDBTest() {
  SQLiteRWDatabase arwdb;
  std::string dbPath = "rwtest.db";
  bool result;

  try {
    arwdb.linkDB(dbPath);
    arwdb.openDB();

    RWDatabase::TruthTable truthTable = 1;

    Gate::SignalList inputs1;
    Gate::Id outputId1;
    GateList varList1;
    std::shared_ptr<GNet> dummy1 = std::make_shared<GNet>
                                   (*makeAnd2(inputs1, outputId1, varList1));
    RWDatabase::GateBindings bindings1 = {{0, inputs1[0].node()},
                                          {1, inputs1[1].node()}};

    Gate::SignalList inputs2;
    Gate::Id outputId2;
    GateList varList2;
    std::shared_ptr<GNet> dummy2 = std::make_shared<GNet>
                                   (*makeOr2(inputs2, outputId2, varList2));
    RWDatabase::GateBindings bindings2 = {{0, inputs2[0].node()},
                                          {1, inputs2[1].node()}};

    dummy1->sortTopologically();
    dummy2->sortTopologically();

    RWDatabase::BoundGNetList bgl = {{dummy1, bindings1}, {dummy2, bindings2}};

    arwdb.insertIntoDB(truthTable, bgl);

    auto newBgl = arwdb.get(truthTable);

    result = areEquivalent(bgl[0], newBgl[0]) &&
             areEquivalent(bgl[1], newBgl[1]);

    arwdb.closeDB();
  } catch (const char* msg) {
    std::cout << msg << std::endl;
  }
  remove(dbPath.c_str());
  return result;
}

bool updateARWDBTest() {
  SQLiteRWDatabase arwdb;
  std::string dbPath = "rwtest.db";
  bool result;

  try {
    arwdb.linkDB(dbPath);
    arwdb.openDB();

    RWDatabase::TruthTable truthTable = 1;

    Gate::SignalList inputs1;
    Gate::Id outputId1;
    GateList varList1;
    std::shared_ptr<GNet> dummy1 = std::make_shared<GNet>
                                   (*makeAnd2(inputs1, outputId1, varList1));
    RWDatabase::GateBindings bindings1 = {{0, inputs1[0].node()},
                                          {1, inputs1[1].node()}};

    Gate::SignalList inputs2;
    Gate::Id outputId2;
    GateList varList2;
    std::shared_ptr<GNet> dummy2 = std::make_shared<GNet>
                                   (*makeOr2(inputs2, outputId2, varList2));
    RWDatabase::GateBindings bindings2 = {{0, inputs2[0].node()},
                                          {1, inputs2[1].node()}};

    dummy1->sortTopologically();
    dummy2->sortTopologically();

    RWDatabase::BoundGNetList bgl = {{dummy1, bindings1}};
    RWDatabase::BoundGNetList newBgl = {{dummy2, bindings2}};

    arwdb.insertIntoDB(truthTable, bgl);

    arwdb.updateInDB(truthTable, {{dummy2, bindings2}});

    auto gottenBgl = arwdb.get(truthTable);

    result = areEquivalent(gottenBgl[0], newBgl[0]);

    arwdb.closeDB();
  } catch (const char* msg) {
    std::cout << msg << std::endl;
  }
  remove(dbPath.c_str());
  return result;
}

bool deleteARWDBTest() {
  SQLiteRWDatabase arwdb;
  std::string dbPath = "rwtest.db";
  bool result;

  try {
    arwdb.linkDB(dbPath);
    arwdb.openDB();

    RWDatabase::TruthTable truthTable = 1;

    Gate::SignalList inputs1;
    Gate::Id outputId1;
    GateList varList1;
    std::shared_ptr<GNet> dummy1 = std::make_shared<GNet>
                                   (*makeAnd2(inputs1, outputId1, varList1));
    RWDatabase::GateBindings bindings1 = {{0, inputs1[0].node()},
                                          {1, inputs1[1].node()}};

    Gate::SignalList inputs2;
    Gate::Id outputId2;
    GateList varList2;
    std::shared_ptr<GNet> dummy2 = std::make_shared<GNet>
                                   (*makeOr2(inputs2, outputId2, varList2));
    RWDatabase::GateBindings bindings2 = {{0, inputs2[0].node()},
                                          {1, inputs2[1].node()}};

    dummy1->sortTopologically();
    dummy2->sortTopologically();

    RWDatabase::BoundGNetList bgl = {{dummy1, bindings1}, {dummy2, bindings2}};

    arwdb.insertIntoDB(truthTable, bgl);
    arwdb.deleteFromDB(truthTable);

    result = !arwdb.contains(truthTable);

    arwdb.closeDB();
  } catch (const char* msg) {
    std::cout << msg << std::endl;
  }
  remove(dbPath.c_str());
  return result;
}

TEST(RWDatabaseTest, BasicTest) {
  EXPECT_TRUE(basicTest());
}

TEST(RWDatabaseTest, InsertGetARWDBTest) {
  EXPECT_TRUE(insertGetARWDBTest());
}

TEST(RWDatabaseTest, UpdateARWDBTest) {
  EXPECT_TRUE(updateARWDBTest());
}

TEST(RWDatabaseTest, DeleteARWDBTest) {
  EXPECT_TRUE(deleteARWDBTest());
}
