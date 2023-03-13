//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "rwdatabase.h"

#include <map>
#include <memory>
#include <vector>

using ARWDatabase = eda::gate::optimizer::ARWDatabase;
using BindedGNet = eda::gate::optimizer::RWDatabase::BindedGNet;
using BindedGNetList = eda::gate::optimizer::RWDatabase::BindedGNetList;
using Gate = eda::gate::model::Gate;
using GateList = std::vector<Gate::Id>;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;
using RWDatabase = eda::gate::optimizer::RWDatabase;

namespace eda::gate::optimizer {

std::string ARWDatabase::serialize(const BindedGNetList &list) {
  std::stringstream ss;
  ss << list.size() << ' ';
  for (auto &bGNet : list) {
    auto bindings = bGNet.bindings;
    auto net = bGNet.net;
    if(!net->isSorted()) {
      throw "Net isn't topologically sorted.";
    }

    ss << bindings.size() << ' ';
    for (auto &binding : bindings) {
      ss << binding.first << ' ' << binding.second << ' ';
    }

    ss << net->gates().size() << ' ';
    for (auto *gate : net->gates()) {
      ss << (uint16_t)gate->func() << ' ' << gate->id() << ' ';
      ss << gate->inputs().size() << ' ';
      for (auto signal : gate->inputs()) {
        ss << signal.node() << ' ';
      }
    }
  }
  return ss.str();
}

BindedGNetList ARWDatabase::deserialize(const std::string &str) {
  std::stringstream ss;
  ss.str(str);
  BindedGNetList result;
  size_t size; ss >> size;
  for (size_t i = 0;i < size;i++) {
    BindedGNet bGNet;

    size_t bindingsSize; ss >> bindingsSize;
    bGNet.bindings = GateBindings();
    auto reversedBindings = ReversedGateBindings();

    for (size_t j = 0;j < bindingsSize;j++) {
      InputId key; Gate::Id value;
      ss >> key >> value;
      bGNet.bindings.insert(std::make_pair<InputId, Gate::Id>
                            (std::forward<InputId>(key),
                            std::forward<Gate::Id>(value)));
      reversedBindings.insert(std::make_pair<Gate::Id, InputId>
                            (std::forward<Gate::Id>(value),
                            std::forward<InputId>(key)));
    }

    size_t gateCount; ss >> gateCount;
    std::shared_ptr<GNet> net = std::make_shared<GNet>();

    GateMap oldNewMap;

    for (size_t j = 0;j < gateCount;j++) {
      GateSymbol::Value func; Gate::Id id;
      size_t inputCount; Gate::SignalList inputs;

      uint16_t intFunc;
      ss >> intFunc >> id >> inputCount;
      func = (GateSymbol::Value)intFunc;

      if (inputCount == 0) {
        Gate::Id newId = net->addGate(func, inputs);
        oldNewMap[id] = newId;
        bGNet.bindings[reversedBindings[id]] = newId;
      } else {
        for (size_t k = 0;k < inputCount;k++) {
          Gate::Id inputId; ss >> inputId;
          Gate::Id newInputId = oldNewMap[inputId];
          inputs.push_back(Gate::Signal::always(newInputId));
        }
        Gate::Id newId = net->addGate(func, inputs);
        oldNewMap[id] = newId;
      }
    }
    bGNet.net = net;
    bGNet.net->sortTopologically();

    result.push_back(bGNet);
  }

  return result;
}

void ARWDatabase::linkDB(const std::string &path) {
  _rc = sqlite3_open(path.c_str(), &_db);

  if (_rc != SQLITE_OK) {
    throw "Can't open database.";
  }

  _selectResult.clear();
  std::string sql = "SELECT name FROM sqlite_master WHERE " \
                    "type='table' AND name='RWDatabase';";
  _rc = sqlite3_exec(_db, sql.c_str(), selectSQLCallback,
                      (void*)(&_selectResult), &_zErrMsg);
  if (_rc != SQLITE_OK) {
    throw "Can't use db.";
  }
  if (_selectResult.empty()) {
    sql = "CREATE TABLE RWDatabase (ValueVector BIGINT PRIMARY KEY,"\
          " BGNet BLOB);";

    _rc = sqlite3_exec(_db, sql.c_str(), dummySQLCallback, 0, &_zErrMsg);

    if (_rc != SQLITE_OK) {
      throw "Can't create table.";
    }
  }

  _pathDB = path;
  _isLinked = true;
  sqlite3_close(_db);
}

void ARWDatabase::openDB() {
  if (!_isLinked) {
    throw "No database was linked.";
  }
  _rc = sqlite3_open(_pathDB.c_str(), &_db);
  if (_rc != SQLITE_OK) {
    throw "Can't open database.";
  }
  _isOpened = true;
}

void ARWDatabase::closeDB() {
  assert(_isLinked);
  sqlite3_close(_db);
  _isOpened = false;
}

bool ARWDatabase::find(const ValueVector &key) {
  if (_storage.find(key) != _storage.end()) {
    return true;
  }
  if (_isOpened) {
    _selectResult.clear();
    std::string sql = "SELECT * FROM RWDatabase WHERE ValueVector = '"
        + std::to_string(key) + "';";
    _rc = sqlite3_exec(_db, sql.c_str(), selectSQLCallback,
                        (void*)(&_selectResult), &_zErrMsg);
    if (_rc != SQLITE_OK) {
      throw "Can't select from db";
    }
    return !(_selectResult.empty());
  }
  return false;
}

BindedGNetList ARWDatabase::get(const ValueVector &key) {
  if (_storage.find(key) != _storage.end()) {
    return _storage[key];
  }
  if (_isOpened) {
    _selectResult.clear();
    std::string sql = "SELECT * FROM RWDatabase WHERE ValueVector = "
        + std::to_string(key) + ";";
    _rc = sqlite3_exec(_db, sql.c_str(), selectSQLCallback,
                        (void*)(&_selectResult), &_zErrMsg);
    if (_rc != SQLITE_OK) {
      throw "Can't select from db";
    }
    if (!_selectResult.empty()) {
      BindedGNetList deser = deserialize(_selectResult[0].second);
      set(key, deser);
      return deser;
    }
  }
  return BindedGNetList();
}

void ARWDatabase::insertIntoDB(const ValueVector &key, const BindedGNetList &value) {
  std::string ser = serialize(value);
  std::string sql = "INSERT INTO RWDatabase(ValueVector, BGNet) " \
                    "VALUES (" + std::to_string(key) + ", '" + ser + "');";
  assert(_isOpened);
  _rc = sqlite3_exec(_db, sql.c_str(), dummySQLCallback, 0, &_zErrMsg);
  if (_rc != SQLITE_OK) {
    std::cout << _zErrMsg << '\n';
    throw "Can't insert value into db.";
  }
}

void ARWDatabase::updateInDB(const ValueVector &key, const BindedGNetList &value) {
  assert(_isOpened);
  std::string ser = serialize(value);
  std::string sql = "UPDATE RWDatabase SET BGNet = '" + ser + "' WHERE "
                    "ValueVector=" + std::to_string(key) + ";";
  _rc = sqlite3_exec(_db, sql.c_str(), dummySQLCallback, 0, &_zErrMsg);
  if (_rc != SQLITE_OK) {
    std::cout << _zErrMsg << '\n';
    throw "Can't update value.";
  }
}

void ARWDatabase::deleteFromDB(const ValueVector &key) {
  assert(_isOpened);
  std::string sql = "DELETE FROM RWDatabase WHERE ValueVector=" +
                    std::to_string(key) + ";";
  _rc = sqlite3_exec(_db, sql.c_str(), dummySQLCallback, 0, &_zErrMsg);
  if (_rc != SQLITE_OK) {
    throw "Can't delete value.";
  }
}

} // namespace eda::gate::optimizer
