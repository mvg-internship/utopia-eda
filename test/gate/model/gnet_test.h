//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

#include <memory>

namespace eda::gate::model {

// (x1 | ... | xN).
std::shared_ptr<GNet> makeOr(const unsigned N,
                             Gate::SignalList &inputs,
                             Gate::Id &outputId);
// (x1 & ... & xN).
std::shared_ptr<GNet> makeAnd(const unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);
// ~(x1 | ... | xN).
std::shared_ptr<GNet> makeNor(const unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);
// ~(x1 & ... & xN).
std::shared_ptr<GNet> makeNand(const unsigned N,
                               Gate::SignalList &inputs,
                               Gate::Id &outputId);
// (~x1 | ... | ~xN).
std::shared_ptr<GNet> makeOrn(const unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);
// (~x1 & ... & ~xN).
std::shared_ptr<GNet> makeAndn(const unsigned N,
                               Gate::SignalList &inputs,
                               Gate::Id &outputId);
// Maj(x1, x2, ..., xN).
std::shared_ptr<GNet> makeMaj(const unsigned N,
                              Gate::SignalList &inputs,
                              Gate::Id &outputId);

// Random hierarchical network.
std::shared_ptr<GNet> makeRand(const std::size_t nGates,
                               const std::size_t nSubnets);

void dump(const GNet &net);

} // namespace eda::gate::model
