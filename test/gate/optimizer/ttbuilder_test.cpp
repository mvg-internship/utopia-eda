//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/transformer/bdd.h"
#include "gate/optimizer/ttbuilder.h"

#include "gtest/gtest.h"

using BoundGNet = eda::gate::optimizer::RWDatabase::BoundGNet;
using Gate = eda::gate::model::Gate;
using GateList = std::vector<Gate::Id>;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;
using RWDatabase = eda::gate::optimizer::RWDatabase;
using TTBuilder = eda::gate::optimizer::TTBuilder;

const uint64_t AND2_TRUTH_TABLE = 18446462598732840960ull;
const uint64_t XOR2_TRUTH_TABLE = 281474976645120ull;
const uint64_t AND6_TRUTH_TABLE = 9223372036854775808ull;

static std::unique_ptr<GNet> makeAnd2(Gate::SignalList &inputs,
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

  outputId = net->addGate(GateSymbol::OUT, {Gate::Signal::always(outputId)});

  net->sortTopologically();

  return net;
}

static std::unique_ptr<GNet> makeAnd6(Gate::SignalList &inputs,
                                      Gate::Id &outputId,
                                      GateList &varList) {
  auto net = std::make_unique<GNet>();

  varList = GateList(6);
  inputs = Gate::SignalList(6);
  for (size_t i = 0; i < 6; i++) {
    varList[i] = net->newGate();
    inputs[i] = Gate::Signal::always(varList[i]);
  }
  outputId = net->addGate(GateSymbol::AND, inputs);

  outputId = net->addGate(GateSymbol::OUT, {Gate::Signal::always(outputId)});

  net->sortTopologically();

  return net;
}

bool twoVarsBuildTest() {
  Gate::SignalList inputs;
  Gate::Id outputId;
  GateList varList;
  BoundGNet bgnet;

  bgnet.net = std::make_shared<GNet>(*makeAnd2(inputs, outputId, varList));
  for (size_t i = 0; i < varList.size(); i++) {
    bgnet.bindings[i + 1] = varList[i];
  }
  RWDatabase::TruthTable andTT = TTBuilder::build(bgnet);

  inputs.clear();
  varList.clear();
  bgnet.net = std::make_shared<GNet>(*makeXor2(inputs, outputId, varList));
  for (size_t i = 0; i < varList.size(); i++) {
    bgnet.bindings[i + 1] = varList[i];
  }
  RWDatabase::TruthTable xorTT = TTBuilder::build(bgnet);

  return andTT == AND2_TRUTH_TABLE && xorTT == XOR2_TRUTH_TABLE;
}

bool and6BuildTest() {
  Gate::SignalList inputs;
  Gate::Id outputId;
  GateList varList;
  BoundGNet bgnet;

  bgnet.net = std::make_shared<GNet>(*makeAnd6(inputs, outputId, varList));
  for (size_t i = 0; i < varList.size(); i++) {
    bgnet.bindings[i + 1] = varList[i];
  }
  RWDatabase::TruthTable andTT = TTBuilder::build(bgnet);

  return andTT == AND6_TRUTH_TABLE;
}

TEST(TTBuilderTest, TwoVarsBuildTest) {
  EXPECT_TRUE(twoVarsBuildTest());
}

TEST(TTBuilderTest, And6BuildTest) {
  EXPECT_TRUE(and6BuildTest());
}
