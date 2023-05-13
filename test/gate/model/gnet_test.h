//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/debugger/checker.h"
#include "gate/model/gnet.h"

#include <memory>

namespace eda::gate::model {

// (x1 | ... | xN).
std::shared_ptr<GNet> makeOr(unsigned N,
                             Gate::SignalList &inputs,
                             Gate::Id &outputId);
// (x1 & ... & xN).
std::shared_ptr<GNet> makeAnd(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);
// ~(x1 | ... | xN).
std::shared_ptr<GNet> makeNor(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);
// ~(x1 & ... & xN).
std::shared_ptr<GNet> makeNand(unsigned N,
                               Gate::SignalList &inputs,
                               Gate::Id &outputId);
// (~x1 | ... | ~xN).
std::shared_ptr<GNet> makeOrn(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);
// (~x1 & ... & ~xN).
std::shared_ptr<GNet> makeAndn(unsigned N,
                               Gate::SignalList &inputs,
                               Gate::Id &outputId);
// Maj(x1, x2, ..., xN).
std::shared_ptr<GNet> makeMaj(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);

// UDP(x1, x2, ..., xN).
std::shared_ptr<GNet> makeUdp(unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);

// Random hierarchical network.
std::shared_ptr<GNet> makeRand(size_t nGates, size_t nSubnets);

} // namespace eda::gate::model
