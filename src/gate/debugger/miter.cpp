//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/miter.h"

namespace eda::gate::debugger {

bool areMiterable(GNet* net1, GNet* net2, Hints &hints) {
  if (net1->nSourceLinks() != net2->nSourceLinks()) {
    CHECK(false) << "Incorrect inputs of nets\n";
    return false;
  }

  for (auto sourceLink : net1->sourceLinks()) {
    if (hints.sourceBinding.get()->find(sourceLink) == hints.sourceBinding.get()->end()) {
      CHECK(false) << "Incorrect inputs in hints\n";
      return false;
    }
  }
  for (auto targetLink : net1->targetLinks()) {
    if (hints.targetBinding.get()->find(targetLink) == hints.sourceBinding.get()->end()) {
      CHECK(false) << "Incorrect outputs in hints\n";
      return false;
    }
  }

  return true;
}

GNet* miter(GNet* net1, GNet* net2, Hints &hints) {
 
  if (not areMiterable(net1, net2, hints)) {
    return nullptr;
  }

  SignalList inputs, xorSignalList;
  GNet* miter = new GNet();
  miter->addNet(*net1);
  miter->addNet(*net2);
  
  for (auto bind : *hints.sourceBinding.get()) {
    GateId newInputId = miter->addIn();
    if (Gate::get(bind.first.target)->func() == GateSymbol::IN) {
      miter->setGate(bind.first.target, GateSymbol::NOP, newInputId);
      miter->setGate(bind.second.target, GateSymbol::NOP, newInputId);
    } else {
      miter->setGate(bind.first.target, Gate::get(bind.first.target)->func(), newInputId);
      miter->setGate(bind.second.target, Gate::get(bind.second.target)->func(), newInputId);
    }
  }

  for (auto bind : *hints.targetBinding.get()) { 
    GateId newOutId = miter->addGate(GateSymbol::XOR, bind.first.source, bind.second.source);
    xorSignalList.push_back(Signal::always(newOutId));
  }

  GateId finalOutId = miter->addOr(xorSignalList);
  miter->addOut(finalOutId);
  miter->setGate(finalOutId, GateSymbol::OR, xorSignalList);
  for (auto bind : *hints.targetBinding.get()) {
    miter->setGate(bind.first.source, GateSymbol::NOP, Gate::get(bind.first.source)->inputs());
    miter->setGate(bind.second.source, GateSymbol::NOP, Gate::get(bind.second.source)->inputs());
  }
  
  miter->sortTopologically();
  return miter;
} 
} // namespace eda::gate::debugger
