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

namespace eda::gate::transformer {

/**
* \brief Converts GNet to BDD (Binary Decision Diagram) form.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class GNetBDDConverter {
  using Gate = eda::gate::model::Gate;
  using GateSymbol = eda::gate::model::GateSymbol;
  using GNet = eda::gate::model::GNet;

public:
  using BDDList = std::vector<BDD>;
  using GateBDDMap = std::map<Gate::Id, BDD>;
  using GateList = std::vector<Gate::Id>;
  using GateUintMap = std::map<Gate::Id, unsigned>;

  // Converts only one gate of the net.
  static BDD convert(const GNet &net, 
                     const Gate::Id gateId, 
                     GateBDDMap &varMap, 
                     const Cudd &manager);
  
  // Converts list of gates of the net.
  static void convertList(const GNet &net, 
                          const GateList &outputList, 
                          BDDList &outputBDDList, 
                          GateBDDMap &varMap, 
                          const Cudd &manager);

private:
  // Apply gate function to BDD list. Returns result BDD.
  static BDD applyGateFunc(const GateSymbol::Value func, 
                           const BDDList &inputList, 
                           const Cudd &manager);

};

} // namespace eda::gate::transformer
