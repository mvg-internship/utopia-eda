//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/optimizer/database/abc.h"
#include "gate/optimizer/rwdatabase.h"
#include "gate/optimizer/rwmanager.h"

namespace eda::gate::optimizer {

void RewriteManager::initialize(const std::string &library) {
  const auto i = db.find(library);
  assert(i == db.end());

  if (library == DEFAULT) {
    auto database = std::make_shared<RWDatabase>();
    initializeAbcRwDatabase(*database);
    db.emplace(library, database);
  }
}

} // namespace eda::gate::optimizer
