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
#include <string>
#include <vector>

using BoundGNet = eda::gate::optimizer::RWDatabase::BoundGNet;
using BoundGNetList = eda::gate::optimizer::RWDatabase::BoundGNetList;
using Gate = eda::gate::model::Gate;
using GateList = std::vector<Gate::Id>;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;
using RWDatabase = eda::gate::optimizer::RWDatabase;
using SQLiteRWDatabase = eda::gate::optimizer::SQLiteRWDatabase;

namespace eda::gate::optimizer {

std::string SQLiteRWDatabase::serialize(const BoundGNetList &list) {
  std::stringstream ss;
  ss << list.size() << ' ';
  for (const auto &bGNet : list) {
    const auto &inputsDelay = bGNet.inputsDelay;
    const auto &bindings = bGNet.bindings;
    auto net = bGNet.net;
    if (!net->isSorted()) {
      throw "Net isn't topologically sorted.";
    }

    ss << bindings.size() << ' ';
    for (const auto &binding : bindings) {
      ss << binding.first << ' ' << binding.second << ' ';
    }

    ss << inputsDelay.size() << ' ';
    for (auto pair : inputsDelay) {
      uint64_t ser = *( (uint64_t*)&pair.second );
      ss << pair.first << ' ' << ser << ' ';
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

BoundGNetList SQLiteRWDatabase::deserialize(const std::string &str) {
  std::stringstream ss;
  ss.str(str);
  BoundGNetList result;
  size_t size;
  ss >> size;
  for (size_t i = 0; i < size; i++) {
    BoundGNet bGNet;

    size_t bindingsSize;
    ss >> bindingsSize;

    bGNet.bindings = GateBindings();
    auto reversedBindings = ReversedGateBindings();

    for (size_t j = 0; j < bindingsSize; j++) {
      InputId key;
      Gate::Id value;

      ss >> key >> value;
      bGNet.bindings.insert(std::make_pair<InputId, Gate::Id>
                            (std::forward<InputId>(key),
                             std::forward<Gate::Id>(value)));
      reversedBindings.insert(std::make_pair<Gate::Id, InputId>
                              (std::forward<Gate::Id>(value),
                               std::forward<InputId>(key)));
    }

    size_t inputsDelayCount;
    ss >> inputsDelayCount;

    for (size_t j = 0; j < inputsDelayCount; j++) {
      InputId key;
      double value;
      uint64_t ser;

      ss >> key >> ser;
      value = *( (double*)&ser );
      bGNet.inputsDelay[key] = value;
    }

    size_t gateCount;
    ss >> gateCount;
    std::shared_ptr<GNet> net = std::make_shared<GNet>();

    GateMap oldNewMap;

    for (size_t j = 0; j < gateCount; j++) {
      GateSymbol::Value func;
      Gate::Id id;
      size_t inputCount;
      Gate::SignalList inputs;
      uint16_t intFunc;

      ss >> intFunc >> id >> inputCount;
      func = (GateSymbol::Value)intFunc;

      if (inputCount) {
        for (size_t k = 0; k < inputCount; k++) {
          Gate::Id inputId;
          ss >> inputId;
          Gate::Id newInputId = oldNewMap[inputId];
          inputs.push_back(Gate::Signal::always(newInputId));
        }
        Gate::Id newId = net->addGate(func, inputs);
        oldNewMap[id] = newId;
      } else {
        Gate::Id newId = net->addGate(func, inputs);
        oldNewMap[id] = newId;
        bGNet.bindings[reversedBindings[id]] = newId;
      }
    }
    bGNet.net = net;
    bGNet.net->sortTopologically();

    result.push_back(bGNet);
  }

  return result;
}

bool SQLiteRWDatabase::dbContainsRWTable() {
  assert(_isOpened);
  _selectResult.clear();
  std::string sql = "SELECT name FROM sqlite_master WHERE " \
                    "type='table' AND name=?;";
  _rc = sqlite3_bind_exec(_db, sql.c_str(), selectSQLCallback,
                          (void*)(&_selectResult),
                          SQLITE_BIND_TEXT(_dbTableName.c_str()),
                          SQLITE_BIND_END);
  if (_rc != SQLITE_OK) {
    throw "Can't use db.";
  }
  return !_selectResult.empty();
}

void SQLiteRWDatabase::linkDB(const std::string &path) {
  _rc = sqlite3_open(path.c_str(), &_db);

  if (_rc != SQLITE_OK) {
    throw "Can't open database.";
  }
  _isOpened = true;

  if (!dbContainsRWTable()) {
    std::string sql = "CREATE TABLE " + _dbTableName + " (" + _dbKeyName + " "
                      + _dbKeyType + " PRIMARY KEY, " + _dbValueName + " " +
                      _dbValueType + ")";
    _rc = sqlite3_exec(_db, sql.c_str(), nullptr, 0, &_zErrMsg);
    if (_rc != SQLITE_OK) {
      std::cout << sqlite3_errmsg(_db) << '\n';
      throw "Can't create table.";
    }
  }

  _pathDB = path;
  _isLinked = true;
  sqlite3_close(_db);
  _isOpened = false;
}

void SQLiteRWDatabase::openDB() {
  if (!_isLinked) {
    throw "No database was linked.";
  }
  _rc = sqlite3_open(_pathDB.c_str(), &_db);
  if (_rc != SQLITE_OK) {
    throw "Can't open database.";
  }
  _isOpened = true;
}

void SQLiteRWDatabase::closeDB() {
  assert(_isLinked);
  sqlite3_close(_db);
  _isOpened = false;
}

bool SQLiteRWDatabase::contains(const TruthTable &key) {
  if (_storage.find(key) != _storage.end()) {
    return true;
  }
  if (_isOpened) {
    _selectResult.clear();
    std::string sql = "SELECT * FROM " + _dbTableName + " " \
                      "WHERE " + _dbKeyName + "=?";
    _rc = sqlite3_bind_exec(_db, sql.c_str(), selectSQLCallback,
                            (void*)(&_selectResult),
                            SQLITE_BIND_INT64(key),
                            SQLITE_BIND_END);
    if (_rc != SQLITE_OK) {
      std::cout << sqlite3_errmsg(_db) << '\n';
      throw "Can't select.";
    }
    return !_selectResult.empty();
  }
  return false;
}

BoundGNetList SQLiteRWDatabase::get(const TruthTable &key) {
  if (_storage.find(key) != _storage.end()) {
    return _storage[key];
  }
  if (_isOpened) {
    _selectResult.clear();
    std::string sql = "SELECT * FROM " + _dbTableName + " WHERE " +
                      _dbKeyName + "=?";
    _rc = sqlite3_bind_exec(_db, sql.c_str(), selectSQLCallback,
                            (void*)(&_selectResult),
                            SQLITE_BIND_INT64(key),
                            SQLITE_BIND_END);
    if (_rc != SQLITE_OK) {
      std::cout << sqlite3_errmsg(_db) << '\n';
      throw "Can't select.";
    }
    if (!_selectResult.empty()) {
      BoundGNetList deser = deserialize(_selectResult[0].second);
      set(key, deser);
      return deser;
    }
  }
  return BoundGNetList();
}

void SQLiteRWDatabase::insertIntoDB(const TruthTable &key,
                                    const BoundGNetList &value) {
  assert(_isOpened);
  std::string ser = serialize(value);

  std::string sql = "INSERT INTO " + _dbTableName + " (" +
                    _dbKeyName + ", " + _dbValueName + ") " +
                    "VALUES (?,?)";
  _rc = sqlite3_bind_exec(_db, sql.c_str(), nullptr, nullptr,
                          SQLITE_BIND_INT64(key),
                          SQLITE_BIND_TEXT(ser.c_str()),
                          SQLITE_BIND_END);
  if (_rc != SQLITE_OK) {
    std::cout << sqlite3_errmsg(_db) << '\n';
    throw "Can't insert.";
  }
}

void SQLiteRWDatabase::updateInDB(const TruthTable &key,
                                  const BoundGNetList &value) {
  assert(_isOpened);
  std::string ser = serialize(value);
  std::string sql = "UPDATE " + _dbTableName + " SET " + _dbValueName +
                    "=? WHERE " + _dbKeyName + "=?";
  _rc = sqlite3_bind_exec(_db, sql.c_str(), nullptr, nullptr,
                          SQLITE_BIND_TEXT(ser.c_str()),
                          SQLITE_BIND_INT64(key),
                          SQLITE_BIND_END);
  if (_rc != SQLITE_OK) {
    std::cout << sqlite3_errmsg(_db) << '\n';
    throw "Can't update.";
  }
}

void SQLiteRWDatabase::deleteFromDB(const TruthTable &key) {
  assert(_isOpened);
  std::string sql = "DELETE FROM " + _dbTableName + " WHERE " + _dbKeyName + "=?";
  _rc = sqlite3_bind_exec(_db, sql.c_str(), nullptr, nullptr,
                          SQLITE_BIND_INT64(key),
                          SQLITE_BIND_END);
  if (_rc != SQLITE_OK) {
    std::cout << sqlite3_errmsg(_db) << '\n';
    throw "Can't delete.";
  }
}

} // namespace eda::gate::optimizer
