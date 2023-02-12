//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

#include "gate/model/gnet.h"

/**
 * \brief Converts GNet scheme representation to hMetis representation
 * \(see "hMETIS A Hypergraph Partitioning Package Version 1.5.3" 
 * \ by George Karypis and Vipin Kumar).
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
class HMetisPrinter {
  std::vector<int> weights;
  std::vector<size_t> eptr;
  std::vector<unsigned int> eind;

public:
  explicit HMetisPrinter(const eda::gate::model::GNet &net);

  std::vector<int> *getWeights() { return &weights; }

  std::vector<size_t> *getEptr() { return &eptr; }

  std::vector<unsigned int> *getEind() { return &eind; }
};
