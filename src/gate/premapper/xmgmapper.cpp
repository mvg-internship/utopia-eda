//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/premapper/xmgmapper.h"

#include <cassert>
#include <unordered_set>

namespace eda::gate::premapper {

using Gate = eda::gate::model::Gate;
using GNet = eda::gate::model::GNet;

//===----------------------------------------------------------------------===//
// XOR/XNOR
//===----------------------------------------------------------------------===//

Gate::Id XmgMapper::mapXor(const Gate::SignalList &newInputs,
                           bool sign, GNet &newNet) const {
  Gate::SignalList inputs(newInputs.begin(), newInputs.end());
  inputs.reserve(2 * newInputs.size() - 1);

  size_t l = 0;
  size_t r = 1;
  while (r < inputs.size()) {
    // XOR(x,y) = XOR(x,y)
    // XNOR(x,y) = NOT(XOR(x,y))
    const auto x = inputs[l];
    const auto y = inputs[r];
    Gate::Id gateId;

    gateId = newNet.addXor(x, y);
    if (!sign) {
      gateId = mapNop({Gate::Signal::always(gateId)}, false, newNet);
    }

    inputs.push_back(Gate::Signal::always(gateId));

    l += 2;
    r += 2;

    sign = true;
  }

  return inputs[l].node();
}

} // namespace eda::gate::premapper
