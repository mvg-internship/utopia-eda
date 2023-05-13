//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/check_cut.h"

namespace eda::gate::optimizer {

  bool isCut(const GateID &gate, const Cut &cut, GateID &failed) {
    std::queue<GateID> bfs;
    bfs.push(gate);
    while (!bfs.empty()) {
      Gate *cur = Gate::get(bfs.front());
      if (cut.find(cur->id()) == cut.end()) {
        if (cur->isSource()) {
          failed = cur->id();
          return false;
        }
        for (auto input: cur->inputs()) {
          bfs.push(input.node());
        }
      }
      bfs.pop();
    }
    return true;
  }

} // namespace eda::gate::optimizer
