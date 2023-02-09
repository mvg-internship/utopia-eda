//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gate.h"
#include "gate/model/gnet.h"

#include "cuddObj.hh"

#include <map>
#include <memory>
#include <vector>

using namespace eda::gate::model;

namespace eda::gate::transformer {

/**
* \brief Converts GNet to BDD (Binary Decision Diagram) form.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class GNetBDDConverter {
public:
  using GateList = std::vector<Gate::Id>;
  using BDDList = std::vector<BDD>;
  using GateBDDMap = std::map<Gate::Id, BDD>;
  using GateUintMap = std::map<Gate::Id, unsigned>;

  // Converts only one gate of the net.
  static BDD convert(const GNet &net, 
                     const Gate::Id gateId, 
                     GateBDDMap &varMap, 
                     const Cudd &manager);
  
  // Converts list of gates of the net.
  static void convertMany(const GNet &net, 
                          const GateList &outputList, 
                          BDDList &outputBDDList, 
                          GateBDDMap &varMap, 
                          const Cudd &manager);

private:
  // Apply gate function to BDD list. Returns result BDD.
  static BDD applyGateFunc(const GateSymbol::Value func, 
                              const BDDList &inputList, 
                              const Cudd &manager);

  // Recursively converts GNet to BDD. 
  static BDD recursiveConversion(const GNet &net, 
                                const Gate::Id gateId, 
                                GateBDDMap &varMap, 
                                GateBDDMap &gateMap, 
                                const Cudd &manager);
};

} // namespace eda::gate::transformer
