//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "hmetis.h"

HMetisPrinter::HMetisPrinter(const eda::gate::model::GNet &net) {

  weights = std::vector<int>(net.nGates(), 1);
  std::vector<bool> involved;

  involved.reserve(net.nGates());
 
  for (eda::gate::model::Gate* gate : net.gates()) {
    if (gate->id() >= involved.size()) {
      involved.resize(gate->id() + 1);
    }
    involved[gate->id()] = true;
  }
  
  std::vector<unsigned int> map(involved.size());
  unsigned int prev = 0;

  for (size_t i = 0; i < involved.size(); ++i) {
    if (involved[i]) {
      map[i] = prev++;
    }
  }

  eptr.push_back(0);
  for (eda::gate::model::Gate* gate : net.gates()) {
    for (const eda::gate::model::Gate::Link& link : gate->links()) {
      eind.push_back(map[link.source]);
      eind.push_back(map[link.target]);
      eptr.push_back(eind.size());
    }
  }
}
