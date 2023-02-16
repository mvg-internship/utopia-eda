//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/checker.h"
#include "gate/model/gnet_test.h"
#include "gate/premapper/migmapper.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <iostream>

// gate(x1, ..., xN).
static std::unique_ptr<GNet> makeNet(GateSymbol gate,
                                     unsigned N,
                                     Gate::SignalList &inputs,
                                     Gate::Id &outputId) {
  auto net = std::make_unique<GNet>();

  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->newGate();
    const Gate::Signal input = Gate::Signal::always(inputId);
    inputs.push_back(input);
  }

  outputId = net->addGate(gate, inputs);
  net->sortTopologically();

  return net;
}

// gate(~x1, ..., ~xN).
static std::unique_ptr<GNet> makeNetn(GateSymbol gate,
                                      unsigned N,
                                      Gate::SignalList &inputs,
                                      Gate::Id &outputId) {
  auto net = std::make_unique<GNet>();

  Gate::SignalList andInputs;
  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->newGate();
    const Gate::Signal input = Gate::Signal::always(inputId);
    inputs.push_back(input);

    const Gate::Id notGateId = net->addGate(GateSymbol::NOT, { input });
    const Gate::Signal andInput = Gate::Signal::always(notGateId);
    andInputs.push_back(andInput);
  }

  outputId = net->addGate(gate, andInputs);
  net->sortTopologically();

  return net;
}

// <x1, ..., xN>.
std::unique_ptr<GNet> makeMaj(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId) {
  return makeNet(GateSymbol::MAJ, N, inputs, outputId);
}

bool equivalence(const std::unique_ptr<GNet> &net, const std::shared_ptr<GNet> &migmapped) {
  using Link = eda::gate::model::Gate::Link;
  using Checker = eda::gate::debugger::Checker;
  using GateBinding = Checker::GateBinding;
  using Hints = Checker::Hints;
  using MigMapper = eda::gate::premapper::MigMapper;
  using GateIdMap = MigMapper::GateIdMap;

  Checker checker;
  GateIdMap oldToNewGates;
  GateBinding imap, omap, tmap;

  for (auto oldSourceLink : net->sourceLinks()) {
    auto newSourceId = oldToNewGates[oldSourceLink.target];
    imap.insert({oldSourceLink, Link(newSourceId)});
  }

  for (auto oldTriggerId : net->triggers()) {
    auto newTriggerId = oldToNewGates[oldTriggerId];
    tmap.insert({Link(oldTriggerId), Link(newTriggerId)});
  }

  // TODO: Here are only triggers.
  for (auto oldTriggerId : net->triggers()) {
    auto newTriggerId = oldToNewGates[oldTriggerId];
    auto *oldTrigger = Gate::get(oldTriggerId);
    auto *newTrigger = Gate::get(newTriggerId);

    auto oldDataId = oldTrigger->input(0).node();
    auto newDataId = newTrigger->input(0).node();

    omap.insert({Link(oldDataId), Link(newDataId)});
  }

  Hints hints;
  hints.sourceBinding = std::make_shared<GateBinding>(std::move(imap));
  hints.targetBinding = std::make_shared<GateBinding>(std::move(omap));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tmap));
  
  return checker.areEqual(*net, *migmapped, hints);
}

void dump(const GNet &net) {
    std::cout << net << '\n';
    std::cout << "N=" << net.nGates() << '\n';
    std::cout << "I=" << net.nSourceLinks() << '\n';
    std::cout << "O=" << net.nTargetLinks() << '\n';
}

TEST(MigMapperTest, MigMapperOrTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOr(2, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperAndTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAnd(2, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMaj3Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(3, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMaj5Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(5, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMaj7Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(7, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMaj9Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(9, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMaj11Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(11, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperMaj17Test) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeMaj(17, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperNorTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNor(2, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperNandTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNand(2, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperOrnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeOrn(2, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}

TEST(MigMapperTest, MigMapperAndnTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeAndn(2, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);

  // equivalence
  bool equal = equivalence(net, migmapped);
  std::cout << "equivalence:" << '\n';
  EXPECT_TRUE(equal);
  EXPECT_TRUE(net != nullptr);
}