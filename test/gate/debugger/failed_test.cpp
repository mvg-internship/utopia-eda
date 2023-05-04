//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet_test.h"
#include "gate/model/gnet.h"
#include "gtest/gtest.h"

using Signal = eda::gate::model::Gate::Signal;
using SignalList = eda::gate::model::Gate::SignalList;
using GateId = eda::gate::model::Gate::Id;
using GateIdList = std::vector<GateId>;

TEST(failedTest, failedTest) {
  auto net = GNet(0);
  SignalList inps;
  int countInp = 100;

  for (int i = 0; i < countInp; i++) {
    GateId z = net.addIn();
    inps.push_back(Signal::always(z));
  }
  GateId y = net.addGate(GateSymbol::OR, inps);
  GateId outId = net.addOut(y);

  auto net1 = GNet(0);

  for (int i = 0; i < countInp; i++) {
    GateId z = net1.addIn();
    inps.push_back(Signal::always(z));
  }
  y = net1.addGate(GateSymbol::OR, inps);
  outId = net1.addOut(y);

  auto net2 = GNet(0);

  GateId id = net2.addIn();
  net2.addNet(net);
  net2.addNet(net1);
  net2.setGate(net2.gates().at(0)->id(), GateSymbol::NOP, id);
  net2.setGate(net2.gates().at(1)->id(), GateSymbol::NOP, id);

}

