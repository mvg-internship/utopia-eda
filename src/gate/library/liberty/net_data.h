//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"
#include "gate/optimizer/rwdatabase.h"

#include <memory>
#include <vector>

struct NetData {

  /**
    * \brief Implements translation from Liberty format file to GNet.
    * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
    */

  std::vector<std::unique_ptr<eda::gate::model::GNet>> combNets;
  std::vector<std::unique_ptr<eda::gate::model::GNet>> memNets;

  void fillDatabase(
      eda::gate::optimizer::RWDatabase &database);

  static std::vector<eda::gate::optimizer::RWDatabase::TruthTable> buildTruthTab(
      const eda::gate::model::GNet *net);

};
