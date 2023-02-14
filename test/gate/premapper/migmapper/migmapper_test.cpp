//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

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
  EXPECT_TRUE(net != nullptr);
}

// TEST(MigMapperTest, MigMapperMaj9Test) {
//   Gate::SignalList inputs;
//   Gate::Id outputId;
//   auto net = makeMaj(9, inputs, outputId);
//   EXPECT_TRUE(net != nullptr);
// }

// TEST(MigMapperTest, MigMapperMaj11Test) {
//   Gate::SignalList inputs;
//   Gate::Id outputId;
//   auto net = makeMaj(11, inputs, outputId);
//   EXPECT_TRUE(net != nullptr);
// }

// TEST(MigMapperTest, MigMapperMaj17Test) {
//   Gate::SignalList inputs;
//   Gate::Id outputId;
//   auto net = makeMaj(17, inputs, outputId);
//   EXPECT_TRUE(net != nullptr);
// }

TEST(MigMapperTest, MigMapperNorTest) {
  Gate::SignalList inputs;
  Gate::Id outputId;
  auto net = makeNor(2, inputs, outputId);
  dump(*net);
  eda::gate::premapper::MigMapper migmapper;
  auto migmapped = migmapper.map(*net);
  dump(*migmapped);
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
  EXPECT_TRUE(net != nullptr);
}