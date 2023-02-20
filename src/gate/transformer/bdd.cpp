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
    for (size_t i = 1; i < inputList.size(); i++) { 
      result = result & inputList[i];
    }
    break;
  case GateSymbol::OR:
    result = inputList[0];
    for (size_t i = 1; i < inputList.size(); i++) { 
      result = result | inputList[i];
    }
    break;
  case GateSymbol::XOR:
    result = inputList[0];
    for (size_t i = 1; i < inputList.size(); i++) { 
      result = result ^ inputList[i];
    }
    break;
  case GateSymbol::NAND:
    result = inputList[0];
    for (size_t i = 1; i < inputList.size(); i++) { 
      result = result & inputList[i];
    }
    result = !result;
    break;
  case GateSymbol::NOR:
    result = inputList[0];
    for (size_t i = 1; i < inputList.size(); i++) { 
      result = result | inputList[i];
    }
    result = !result;
    break;
  case GateSymbol::XNOR:
    result = inputList[0];
    for (size_t i = 1; i < inputList.size(); i++) { 
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

void GNetBDDConverter::convertList(const GNet &net,
                                   const GateList &gateList, 
                                   BDDList &outputBDDList, 
                                   GateBDDMap &varMap, 
                                   const Cudd &manager) {
  BDDList result = BDDList(gateList.size());
  assert(net.isSorted());

  GateUintMap gateIndexMap;
  for (size_t i = 0; i < gateList.size(); i++) {
    gateIndexMap[gateList[i]] = i;
  }

  GateBDDMap gateMap;
  for (auto *gate : net.gates()) {
    Gate::Id gateId = gate->id(); 

    BDD resultBDD;
    if (gate->isSource()) {
      assert(varMap.find(gateId) != varMap.end());
      resultBDD = varMap[gateId];
    } else {
      BDDList inputBDDList;
      for (auto signal : gate->inputs()) {
        Gate::Id inputId = signal.node();
        BDD inputBDD = gateMap[inputId];
        inputBDDList.push_back(inputBDD);
      }
      resultBDD = applyGateFunc(gate->func(), inputBDDList, manager);
    }

    gateMap[gateId] = resultBDD;
    if (gateIndexMap.find(gateId) != gateIndexMap.end()) {
      result[gateIndexMap[gateId]] = resultBDD;
    }
  }

  outputBDDList = result;
}

BDD GNetBDDConverter::convert(const GNet &net, 
                              const Gate::Id gateId, 
                              GateBDDMap &varMap, 
                              const Cudd &manager) {
  BDDList resultList;
  convertList(net, {gateId}, resultList, varMap, manager);
  return resultList[0];
}

} // namespace eda::gate::transformer
