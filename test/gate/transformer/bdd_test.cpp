//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/transformer/bdd.h"

#include "gtest/gtest.h"

using BDDList = eda::gate::transformer::GNetBDDConverter::BDDList;
using Gate = eda::gate::model::Gate;
using GateBDDMap = eda::gate::transformer::GNetBDDConverter::GateBDDMap;
using GateList = eda::gate::transformer::GNetBDDConverter::GateList;
using GateSymbol = eda::gate::model::GateSymbol;
using GateUintMap = eda::gate::transformer::GNetBDDConverter::GateUintMap;
using GNet = eda::gate::model::GNet;
using GNetBDDConverter = eda::gate::transformer::GNetBDDConverter;

// x0 & x1
static std::unique_ptr<GNet> makeAnd2(Gate::SignalList
                                      &inputs,
                                      Gate::Id &outputId,
                                      GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0Id = net->newGate(), x1Id = net->newGate();
  Gate::SignalList and0Inputs = {Gate::Signal::always(x0Id),
                                 Gate::Signal::always(x1Id)};

  inputs = and0Inputs;
  outputId = net->addGate(GateSymbol::AND, and0Inputs);
  varList = {x0Id, x1Id};

  net->sortTopologically();

  return net;
}

// x0 | x1
static std::unique_ptr<GNet> makeOr2(Gate::SignalList &inputs,
                                     Gate::Id &outputId,
                                     GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0Id = net->newGate(), x1Id = net->newGate();
  Gate::SignalList or0Inputs = {Gate::Signal::always(x0Id),
                                Gate::Signal::always(x1Id)};

  inputs = or0Inputs;
  outputId = net->addGate(GateSymbol::OR, or0Inputs);
  varList = {x0Id, x1Id};

  net->sortTopologically();

  return net;
}

// x0 ^ x1
static std::unique_ptr<GNet> makeXor2(Gate::SignalList &inputs,
                                      Gate::Id &outputId,
                                      GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0Id = net->newGate(), x1Id = net->newGate();
  Gate::SignalList xor0Inputs = {Gate::Signal::always(x0Id),
                                 Gate::Signal::always(x1Id)};

  inputs = xor0Inputs;
  outputId = net->addGate(GateSymbol::XOR, xor0Inputs);
  varList = {x0Id, x1Id};

  net->sortTopologically();

  return net;
}

bool transformerAndTest() {
  Gate::SignalList inputs; Gate::Id outputId; GateList varList;
  std::unique_ptr<GNet> net = makeAnd2(inputs, outputId, varList);

  Cudd manager(0, 0);
  BDDList x = { manager.bddVar(), manager.bddVar() };
  GateBDDMap varMap;
  for (int i = 0; i < 2; i++) {
    varMap[inputs[i].node()] = x[i];
  }

  BDD netBDD = GNetBDDConverter::convert(*net, outputId, varMap, manager);
  BDD andBDD = x[0] & x[1];

  return netBDD == andBDD;
}

bool transformerOrTest() {
  Gate::SignalList inputs; Gate::Id outputId; GateList varList;
  std::unique_ptr<GNet> net = makeXor2(inputs, outputId, varList);

  Cudd manager(0, 0);
  BDDList x = { manager.bddVar(), manager.bddVar() };
  GateBDDMap varMap;
  for (int i = 0; i < 2; i++) {
    varMap[inputs[i].node()] = x[i];
  }

  BDD netBDD = GNetBDDConverter::convert(*net, outputId, varMap, manager);
  BDD xorBDD = x[0] ^ x[1];

  return netBDD == xorBDD;
}

bool transformerNorTest() {
  Gate::SignalList inputs; Gate::Id outputId1; GateList varList;
  std::unique_ptr<GNet> net = makeOr2(inputs, outputId1, varList);

  Cudd manager(0, 0);
  BDDList x = { manager.bddVar(), manager.bddVar() };
  GateBDDMap varMap;
  for (int i = 0; i < 2; i++) {
    varMap[inputs[i].node()] = x[i];
  }

  Gate::Id outputId2 = net->addGate(GateSymbol::NOT,
                                    {Gate::Signal::always(outputId1)});

  BDDList result;

  net->sortTopologically();
  GNetBDDConverter::convertList(*net, {outputId1, outputId2},
                                result, varMap, manager);
  BDD orBDD = x[0] | x[1];
  BDD norBDD = !(orBDD);

  return result[0] == orBDD && result[1] == norBDD;
}

TEST(TransformerBDDGNetTest, TransformerAndTest) {
  EXPECT_TRUE(transformerAndTest());
}

TEST(TransformerBDDGNetTest, TransformerOrTest) {
  EXPECT_TRUE(transformerOrTest());
}

TEST(TransformerBDDGNetTest, TransformerNorTest) {
  EXPECT_TRUE(transformerNorTest());
}
