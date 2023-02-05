//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gate.h"
#include "gate/model/gnet.h"

// #include "cudd.h"
#include "cuddObj.hh"

#include <iostream>
#include <map>
#include <memory>
#include <vector>

using namespace eda::gate::model;

namespace eda::gate::transformer {

/**
* \brief Convert GNet to BDD
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class GNetBDDConverter {
public:
  using GateList = std::vector<Gate::Id>;
  using BDDList = std::vector<BDD>;
  using GateBDDMap = std::map<Gate::Id, BDD>;
  using GateUintMap = std::map<Gate::Id, unsigned>;

  // Converts only one output of the net.
  static BDD convertSingleOutput(const GNet &net, 
                                const Gate::Id outputId, 
                                GateBDDMap &varMap, 
                                const Cudd &manager);
  
  // Converts list of outputs of the net.
  static void convertMultipleOutputs(const GNet &net, 
                                    const GateList &outputList, 
                                    BDDList &outputBDDList, 
                                    GateBDDMap &varMap, 
                                    const Cudd &manager);

private:
  // Apply gate operation to BDD list. Returns result BDD.
  static BDD gateOperationBDD(const GateSymbol::Value func, 
                              const BDDList &inputList, 
                              const Cudd &manager);

  // Recursively converts GNet to BDD. 
  static BDD recursiveConversion(const GNet &net, 
                                const Gate::Id outputId, 
                                GateBDDMap &varMap, 
                                GateBDDMap &gateMap, 
                                const Cudd &manager);
};

} // namespace eda::gate::transformer
