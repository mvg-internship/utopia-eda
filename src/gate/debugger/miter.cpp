//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/miter.h"

namespace eda::gate::debugger {

bool isMiterable(GNet* net1, GNet* net2, Hints &hints){
  if (net1->nSourceLinks() != net2->nSourceLinks()) {
    std::cout << "Incorrect inputs of nets\n";
    return false;
  }
  if (net1->nTargetLinks() != net2->nTargetLinks()) {
    std::cout << "Incorrect outputs of nets\n";
    return false;
  }

  for (auto sourceLink : net1->sourceLinks()) {
    if (hints.sourceBinding.get()->find(sourceLink) == hints.sourceBinding.get()->end()) {
      std::cout << "Incorrect inputs in hints\n";
      return false;
    }
  }
  for (auto targetLink : net1->targetLinks()) {
    if (hints.targetBinding.get()->find(targetLink) == hints.sourceBinding.get()->end()) {
      std::cout << "Incorrect outputs in hints\n";
      return false;
    }
  }

  return true;
}

GNet* miter(GNet* net1, GNet* net2, Hints &hints){
 
  if (not isMiterable(net1, net2, hints)) {
    return nullptr;
  }

  SignalList inputs;
  SignalList xorSignalList;
  GNet *miter = new GNet();
  miter->addNet(*net1);
  miter->addNet(*net2);
 
  for (auto bind : *hints.sourceBinding.get()) {
    for (unsigned i = 0; i < Gate::get(bind.first.source)->arity(); i++) {
      const GateId inputId = miter->addIn(); 
      inputs.push_back(Signal::always(inputId));
    }
    GateId newInputId = miter->addGate(GateSymbol::IN, inputs);
    miter->setGate(Gate::get(bind.first.source)->id(), GateSymbol::NOP, newInputId);
    miter->setGate(Gate::get(bind.second.source)->id(), GateSymbol::NOP, newInputId);
  }

  for (auto bind : *hints.targetBinding.get()) {
    GateId newOutId = miter->addGate(GateSymbol::XOR, bind.first.target, bind.second.target);
    xorSignalList.push_back(Signal::always(newOutId));
    miter->setGate(bind.first.target, GateSymbol::NOP, newOutId);
    miter->setGate(bind.second.target, GateSymbol::NOP, newOutId);
  }

  GateId finalOutId = miter->addGate(GateSymbol::OR, xorSignalList);
  miter->addOut(finalOutId);

  for (auto xorSignal : xorSignalList) {
    miter->setGate(xorSignal.node(), GateSymbol::NOP, finalOutId); 
  }

  return miter;
} 
} // namespace eda::gate::debugger


 
