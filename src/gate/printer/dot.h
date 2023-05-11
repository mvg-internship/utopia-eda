//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "gate/model/gsymbol.h"

#include <fstream>

class Dot {
public:
  using GNet = eda::gate::model::GNet;
  using Gate = eda::gate::model::Gate;
  using GateSymbol = eda::gate::model::GateSymbol;

  Dot(const GNet *gNet);
  void print(const std::string &filename) const;
  void print(std::ofstream &stream) const;

private:
  const GNet *gNet;
  void print(std::ofstream &stream, const Gate *gate) const;
  static std::vector<std::string> funcNames;
};

