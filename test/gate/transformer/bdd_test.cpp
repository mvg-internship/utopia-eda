//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/transformer/bdd.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace eda::gate::model;
using namespace eda::gate::transformer;

using GateList = GNetBDDConverter::GateList;
using BDDList = GNetBDDConverter::BDDList;
using GateBDDMap = GNetBDDConverter::GateBDDMap;
using GateUintMap = GNetBDDConverter::GateUintMap;

// x0 & x1
static std::unique_ptr<GNet> makeAnd2(Gate::SignalList 
                                  &inputs, 
                                  Gate::Id &outputId, 
                                  GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0_id = net->newGate(), x1_id = net->newGate();
  Gate::SignalList and0_inputs = {Gate::Signal::always(x0_id),
                                  Gate::Signal::always(x1_id)};

  inputs = and0_inputs;
  outputId = net->addGate(GateSymbol::AND, and0_inputs);
  varList = {x0_id, x1_id};

  return net;
}

// x0 | x1
static std::unique_ptr<GNet> makeOr2(Gate::SignalList &inputs, 
                                Gate::Id &outputId, 
                                GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0_id = net->newGate(), x1_id = net->newGate();
  Gate::SignalList or0_inputs = {Gate::Signal::always(x0_id), 
                                  Gate::Signal::always(x1_id)};

  inputs = or0_inputs;
  outputId = net->addGate(GateSymbol::OR, or0_inputs);
  varList = {x0_id, x1_id};

  return net;
}

// x0 ^ x1
static std::unique_ptr<GNet> makeXor2(Gate::SignalList &inputs, 
                                  Gate::Id &outputId, 
                                  GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0_id = net->newGate(), x1_id = net->newGate();
  Gate::SignalList xor0_inputs = {Gate::Signal::always(x0_id),
                                  Gate::Signal::always(x1_id)};

  inputs = xor0_inputs;
  outputId = net->addGate(GateSymbol::XOR, xor0_inputs);
  varList = {x0_id, x1_id};

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
  
  BDD netBDD = GNetBDDConverter::convertSingleOutput(*net, outputId, varMap, 
                                                     manager);
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
  
  BDD netBDD = GNetBDDConverter::convertSingleOutput(*net, outputId, varMap, 
                                                     manager);
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
  GNetBDDConverter::convertMultipleOutputs(*net, {outputId1, outputId2}, 
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
