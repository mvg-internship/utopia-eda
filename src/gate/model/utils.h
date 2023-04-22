//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

namespace eda::gate::model {

inline Gate::SignalList getNewInputs(const Gate::SignalList &oldInputs,
                                     const GNet::GateIdMap &oldToNewGates) {
  Gate::SignalList newInputs(oldInputs.size());

  for (size_t i = 0; i < oldInputs.size(); i++) {
    auto oldInput = oldInputs[i];
    auto newInput = oldToNewGates.find(oldInput.node());
    assert(newInput != oldToNewGates.end());

    newInputs[i] = Gate::Signal(oldInput.event(), newInput->second);
  }

  return newInputs;
}

inline Gate::SignalList getNewInputs(const Gate &oldGate,
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
      assert(i != oldToNewGates.end());

      const auto newInputId = i->second;
      newInputs.push_back(Gate::Signal::always(newInputId));
    }
  }

  return newInputs;
}

} // namespace eda::gate::model