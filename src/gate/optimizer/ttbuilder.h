//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/optimizer/rwdatabase.h"

#include <unordered_map>
#include <memory>
#include <vector>

namespace eda::gate::optimizer {

/**
* \brief Builds truth table for GNet.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class TTBuilder {
  using BoundGNet = RWDatabase::BoundGNet;
  using Gate = eda::gate::model::Gate;
  using GateSymbol = eda::gate::model::GateSymbol;
  using GNet = eda::gate::model::GNet;

public:
  using GateBindings = RWDatabase::GateBindings;
  using ReversedGateBindings = RWDatabase::ReversedGateBindings;
  using TruthTable = RWDatabase::TruthTable;
  using TruthTableList = std::vector<TruthTable>;

  // Function builds truth table this way:
  // x1 x2 x3 x4 x5 x6  F
  //  0  0  0  0  0  0  a0  - 0th bit of TruthTable
  //  0  0  0  0  0  1  a1  - 1st bit of TruthTable
  //  0  0  0  0  1  0  a2  - 2nd bit of TruthTable
  //      ...             ...
  //  1  1  1  1  1  1  a63 - 63th bit of TruthTable
  static TruthTable build(const BoundGNet &bgnet);

private:
  static TruthTable applyGateFunc(const GateSymbol::Value func,
                                  const TruthTableList &inputList);

  // Build truth table for N'th variable.
  static uint64_t buildNthVar(int n);
};

} // namespace eda::gate::optimizer
