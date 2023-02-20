//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gate.h"
#include "gate/model/gnet.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace eda::gate::rwdatabase {

/**
* \brief Implements storage that contains GNets for rewriting, lazy strategy.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class LazyRWDatabase : public RWDatabase {
public:
  override BindedGNetList get(const ValueVector key) {
    if (_storage.find(key) == _storage.end()) {
      synthesizeNew(key);
    }
    return _storage[key];
  }

private:
  // Function that is called if we don't have any GNet's
  // in storage for specified key.
  void synthesizeNew(ValueVector key);
};

} // namespace eda::gate::rwdatabase
