//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/transformer/bdd.h"

#include "cuddObj.hh"

#include <map>
#include <memory>
#include <vector>

using namespace eda::gate::model;

namespace eda::gate::transformer {

BDD GNetBDDConverter::applyGateFunc(const GateSymbol::Value func, 
                                    const BDDList &inputList,
                                    const Cudd &manager) {
  BDD result;
  switch (func) {
  case GateSymbol::ZERO:
    result = manager.bddZero();
    break;
  case GateSymbol::ONE:
    result = manager.bddOne();
    break;
  case GateSymbol::NOP:
    assert(inputList.size() == 1);
    result = inputList[0];
    break;
  case GateSymbol::NOT:
    assert(inputList.size() == 1);
    result = !inputList[0];
    break;
  case GateSymbol::AND:
    result = inputList[0];
    for (int i = 1; i < inputList.size(); i++) { 
      result = result & inputList[i];
    }
    break;
  case GateSymbol::OR:
    result = inputList[0];
    for (int i = 1; i < inputList.size(); i++) { 
      result = result | inputList[i];
    }
    break;
  case GateSymbol::XOR:
    result = inputList[0];
    for (int i = 1; i < inputList.size(); i++) { 
      result = result ^ inputList[i];
    }
    break;
  case GateSymbol::NAND:
    result = inputList[0];
    for (int i = 1; i < inputList.size(); i++) { 
      result = result & inputList[i];
    }
    result = !result;
    break;
  case GateSymbol::NOR:
    result = inputList[0];
    for (int i = 1; i < inputList.size(); i++) { 
      result = result | inputList[i];
    }
    result = !result;
    break;
  case GateSymbol::XNOR:
    result = inputList[0];
    for (int i = 1; i < inputList.size(); i++) { 
      result = result ^ inputList[i];
    }
    result = !result;
    break;
  default:
    assert(false && "Unsupported gate");
    break;
  }
  return result;
}

BDD GNetBDDConverter::recursiveConversion(const GNet &net,
                                          const Gate::Id gateId,
                                          GateBDDMap &varMap,
                                          GateBDDMap &gateMap, 
                                          const Cudd &manager) {
  if (gateMap.find(gateId) != gateMap.end()) {
    return gateMap[gateId];
  }

  Gate *gate = Gate::get(gateId);

  if (gate->isSource()) {
    assert(varMap.find(gateId) != varMap.end());
    gateMap[gateId] = varMap[gateId];
    return varMap[gateId];
  }

  BDDList inputList;
  for (auto signal : gate->inputs()) {
    Gate::Id inputId = signal.node();
    BDD inputBDD = recursiveConversion(net, inputId, varMap, gateMap, manager);
    inputList.push_back(inputBDD);
  }
  BDD result = gateOperationBDD(gate->func(), inputList, manager);
  gateMap[gateId] = result;
  return result;
}

BDD GNetBDDConverter::convert(const GNet &net, 
                              const Gate::Id gateId, 
                              GateBDDMap &varMap, 
                              const Cudd &manager) {
  GateBDDMap gateMap;
  return recursiveConversion(net, gateId, varMap, gateMap, manager);
}

void GNetBDDConverter::convertMany(const GNet &net,
                                   const GateList &outputList, 
                                   BDDList &outputBDDList, 
                                   GateBDDMap &varMap, 
                                   const Cudd &manager) {
  BDDList result = BDDList(outputList.size());
  assert(net.isSorted());

  GateUintMap gateIndexMap;
  for (int i = 0; i < outputList.size(); i++) {
    gateIndexMap[outputList[i]] = i;
  }

  GateBDDMap gateMap;
  for (auto *gate : net.gates()) {
    Gate::Id gateId = gate->id(); 

    BDD resultBDD;
    if (gate->isSource()) {
      assert(varMap.find(gateId) != varMap.end());
      resultBDD = varMap[gateId];
    }
    else {
      BDDList inputBDDList;
      for (auto signal : gate->inputs()) {
        Gate::Id inputId = signal.node();
        BDD inputBDD = gateMap[inputId];
        inputBDDList.push_back(inputBDD);
      }
      resultBDD = gateOperationBDD(gate->func(), inputBDDList, manager);
    }

    gateMap[gateId] = resultBDD;
    if (gateIndexMap.find(gateId) != gateIndexMap.end()) {
      result[gateIndexMap[gateId]] = resultBDD;
    }
  }

  outputBDDList = result;
}

} // namespace eda::gate::transformer
