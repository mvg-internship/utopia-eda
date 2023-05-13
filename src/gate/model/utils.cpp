//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/utils.h"

namespace eda::gate::model {

Gate::SignalList getNewInputs(const Gate::SignalList &oldInputs,
                              const GNet::GateIdMap &oldToNewGates) {
  Gate::SignalList newInputs(oldInputs.size());

  for (size_t i = 0; i < oldInputs.size(); i++) {
    auto oldInput = oldInputs[i];
    auto newInput = oldToNewGates.find(oldInput.node());
    assert((newInput != oldToNewGates.end()) && "The gate was not found");

    newInputs[i] = Gate::Signal(oldInput.event(), newInput->second);
  }

  return newInputs;
}

Gate::SignalList getNewInputs(const Gate &oldGate,
                              const GNet::GateIdMap &oldToNewGates,
                              size_t &n0,
                              size_t &n1) {
  const auto k = oldGate.arity();

  Gate::SignalList newInputs;
  newInputs.reserve(k);

  n0 = 0;
  n1 = 0;
  for (const auto input : oldGate.inputs()) {
    if (model::isValue(input)) {
      const auto isZero = model::isZero(input);
      n0 += (isZero ? 1 : 0);
      n1 += (isZero ? 0 : 1);
    } else {
      const auto i = oldToNewGates.find(input.node());
      assert((i != oldToNewGates.end()) && "The gate was not found");

      const auto newInputId = i->second;
      newInputs.push_back(Gate::Signal::always(newInputId));
    }
  }

  return newInputs;
}

void dump(const GNet &net) {
  std::cout << net << '\n';

  for (auto source : net.sourceLinks()) {
    const auto *gate = Gate::get(source.target);
    std::cout << *gate << std::endl;
  }
  for (auto target : net.targetLinks()) {
    const auto *gate = Gate::get(target.source);
    std::cout << *gate << std::endl;
  }

  std::cout << std::endl;
  std::cout << "N=" << net.nGates() << '\n';
  std::cout << "I=" << net.nSourceLinks() << '\n';
  std::cout << "O=" << net.nTargetLinks() << '\n';
}

} // namespace eda::gate::model
