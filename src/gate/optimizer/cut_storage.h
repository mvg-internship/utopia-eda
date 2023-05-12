//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

#include <unordered_map>

namespace eda::gate::optimizer {
/**
 * \brief Class for storing cuts.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  struct CutStorage {
    using GNet = model::GNet;
    using GateID = GNet::GateId;

    struct HashFunction {
      size_t operator()(const std::unordered_set<GateID> &set) const {
        std::hash<int> hasher;
        size_t answer = 0;

        for (int i: set) {
          answer ^= hasher(i) + 0x9e3779b9 +
                    (answer << 6) + (answer >> 2);
        }
        return answer;
      }
    };

    using Cut = std::unordered_set<GateID>;
    using Cuts = std::unordered_set<Cut, HashFunction>;

    std::unordered_map<GateID, Cuts> cuts;
  };
} // namespace eda::gate::optimizer
