//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "ttbuilder.h"

namespace eda::gate::optimizer {

TTBuilder::TruthTable TTBuilder::applyGateFunc(const GateSymbol::Value func,
                                               const TruthTableList
                                               &inputList) {
  TruthTable result;
  size_t counters[32] = {0};
  size_t inputSize = inputList.size();
  switch (func) {
  case GateSymbol::ZERO:
    result = 0;
    break;
  case GateSymbol::ONE:
    result = -1;
    break;
  case GateSymbol::NOP:
    assert(inputSize == 1);
    result = inputList[0];
    break;
  case GateSymbol::IN:
    assert(inputSize == 1);
    result = inputList[0];
    break;
  case GateSymbol::OUT:
    assert(inputSize == 1);
    result = inputList[0];
    break;
  case GateSymbol::NOT:
    assert(inputSize == 1);
    result = ~inputList[0];
    break;
  case GateSymbol::AND:
    result = inputList[0];
    for (size_t i = 1; i < inputSize; i++) {
      result = result & inputList[i];
    }
    break;
  case GateSymbol::OR:
    result = inputList[0];
    for (size_t i = 1; i < inputSize; i++) {
      result = result | inputList[i];
    }
    break;
  case GateSymbol::XOR:
    result = inputList[0];
    for (size_t i = 1; i < inputSize; i++) {
      result = result ^ inputList[i];
    }
    break;
  case GateSymbol::NAND:
    result = inputList[0];
    for (size_t i = 1; i < inputSize; i++) {
      result = result & inputList[i];
    }
    result = ~result;
    break;
  case GateSymbol::NOR:
    result = inputList[0];
    for (size_t i = 1; i < inputSize; i++) {
      result = result | inputList[i];
    }
    result = ~result;
    break;
  case GateSymbol::XNOR:
    result = inputList[0];
    for (size_t i = 1; i < inputSize; i++) {
      result = result ^ inputList[i];
    }
    result = ~result;
    break;
  case GateSymbol::MAJ:
    if (inputSize == 1) {
      result = inputList[0];
      break;
    }
    if (inputSize == 3) {
      for (size_t i = 0; i < inputSize; ++i) {
        for (size_t j = 0; j < 8; ++j) {
          if (((inputList[i] >> j) & 1) != 0 ) {
            ++counters[j];
          }
        }
      }
      result = 0;
      for (size_t j = 0; j < 8; ++j) {
        if (counters[j] > 1) {
          result |= (1 << j);
        }
      }
      break;
    }
    if (inputSize == 5) {
      for (size_t i = 0; i < inputSize; ++i) {
        for (size_t j = 0; j < 32; ++j) {
          if (((inputList[i] >> j) & 1) != 0 ) {
            ++counters[j];
          }
        }
      }
      result = 0;
      for (size_t j = 0; j < 32; ++j) {
        if (counters[j] > 1) {
          result |= (1 << j);
        }
      }
      break;
    }
    result = inputList[0];
    for (size_t i = 1; i < inputSize; i++) {
      result = result & inputList[i];
    }
    break;
  default:
    assert(false && "Unsupported gate");
    break;
  }
  return result;
}

TTBuilder::TruthTable TTBuilder::build(const BoundGNet &bgnet) {
  TruthTable result = 0;
  if (bgnet.net->isSorted() == false) {
    bgnet.net->sortTopologically();
  }
  ReversedGateBindings rBindings;
  for (const auto &p : bgnet.bindings) {
    rBindings[p.second] = p.first;
  }

  std::unordered_map<Gate::Id, TruthTable> ttMap;
  for (auto *gate : bgnet.net->gates()) {
    Gate::Id gateId = gate->id();

    TruthTable curResult;
    if (gate->isSource()) {
      assert(rBindings.find(gateId) != rBindings.end());
      curResult = buildNthVar(rBindings[gateId]);
    } else {
      TruthTableList inputList;
      for (auto signal : gate->inputs()) {
        Gate::Id inputId = signal.node();
        TruthTable inputTT = ttMap[inputId];
        inputList.push_back(inputTT);
      }
      curResult = applyGateFunc(gate->func(), inputList);
    }

    if (gate->isTarget()) {
      result = curResult;
    }

    ttMap[gateId] = curResult;
  }

  return result;
}

uint64_t TTBuilder::buildNthVar(int n) {
  uint64_t result = 0;
  int div = (1 << n);
  for (uint64_t i = 0; i < 64; i++) {
    if ((i / div) % 2 == 1) {
      result = result | (1ull << i);
    }
  }
  return result;
}

} // namespace eda::gate::optimizer
