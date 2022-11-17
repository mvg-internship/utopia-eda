//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

#include "gnet.h"

class FormatterHMetis {
  std::vector<int> weights;
  std::vector<size_t> eptr;
  std::vector<unsigned int> eind;

public:
  explicit FormatterHMetis(const eda::gate::model::GNet &net);

  inline std::vector<int> *getWeights() { return &weights; }

  inline std::vector<size_t> *getEptr() { return &eptr; }

  inline std::vector<unsigned int> *getEind() { return &eind; }
};
