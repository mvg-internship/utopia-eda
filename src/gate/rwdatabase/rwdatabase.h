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
* \brief Implements storage that contains GNets for rewriting.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class RWDatabase {
  using Gate = eda::gate::model::Gate;
  using GNet = eda::gate::model::GNet;

public:
  using InputId = uint32_t;
  using ValueVector = uint64_t;

  using GateBinding = std::unordered_map<InputId, Gate::Id>;

  struct BindedGNet {
    std::shared_ptr<GNet> net;
    GateBinding binding;
  };

  using BindedGNetList = std::vector<BindedGNet>;

  BindedGNetList get(const ValueVector key) {
    if (_storage.find(key) == _storage.end()) {
      return BindedGNetList();
    }
    return _storage[key];
  }

  void insert(ValueVector key, BindedGNetList value) {
    _storage[key] = value;
  }

  void erase(ValueVector key) {
    _storage.erase(key);
  }

  bool empty() {
    return _storage.empty();
  }

private:
  std::unordered_map<ValueVector, BindedGNetList> _storage;

};

} // namespace eda::gate::rwdatabase
