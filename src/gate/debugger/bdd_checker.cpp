//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/bdd_checker.h"

namespace eda::gate::debugger {

bool bddChecker(GNet *net1, GNet *net2, Hints &hints) {

  GNet *miterNet = miter(net1, net2, hints);
  SignalList inputs;
  GateId outputId = 0;

  Cudd manager(0, 0);
  for (Gate *gate : miterNet->gates()) {
    if (gate->func() == GateSymbol::IN) {
      inputs.push_back(Signal::always(gate->id()));
    }
    if (gate->func() == GateSymbol::OUT) {
      outputId = gate->id();
    }
  }
  BDDList x = {};
  for (unsigned i = 0; i < inputs.size(); i++) {
    x.push_back(manager.bddVar());
  }
  
  GateBDDMap varMap;
  for (unsigned i = 0; i < x.size(); i++) {
    varMap[inputs[i].node()] = x[i];
  }

  BDD netBDD = GNetBDDConverter::convert(*miterNet, outputId, varMap, manager);
  if (netBDD == manager.bddZero()) {
    return true;
  }
  return false;
} 
} // namespace eda::gate::debugger
