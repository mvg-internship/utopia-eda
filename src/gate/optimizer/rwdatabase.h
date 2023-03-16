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

#include "sqlite3.h"

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace eda::gate::optimizer {

/**
* \brief Implements storage that contains GNets for rewriting.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class RWDatabase {

public:
  using Gate = eda::gate::model::Gate;
  using GateMap = std::unordered_map<Gate::Id, Gate::Id>;
  using GateSymbol = eda::gate::model::GateSymbol;
  using GNet = eda::gate::model::GNet;

  // "Virtual" input gate id.
  using InputId = uint32_t;
  // Contain value vector of boolean function.
  // Represents boolean function of 6 variables.
  using TruthTable = uint64_t;

  // Binds "virtual" input ids to real primary input ids of GNet.
  using GateBindings = std::unordered_map<InputId, Gate::Id>;
  using ReversedGateBindings = std::unordered_map<Gate::Id, InputId>;

  struct BindedGNet {
    std::shared_ptr<GNet> net;
    GateBindings bindings;
  };

  using BindedGNetList = std::vector<BindedGNet>;

  // Basic interface.
  virtual bool find(const TruthTable &key) {
    return (_storage.find(key) != _storage.end());
  }

  virtual BindedGNetList get(const TruthTable &key) {
    if (!find(key)) {
      return BindedGNetList();
    }
    return _storage[key];
  }

  virtual void set(const TruthTable &key, const BindedGNetList &value) {
    _storage[key] = value;
  }

  virtual void erase(const TruthTable &key) {
    _storage.erase(key);
  }

  virtual bool empty() {
    return _storage.empty();
  }

protected:
  std::unordered_map<TruthTable, BindedGNetList> _storage;

};

/**
* \brief Implements storage that contains GNets for rewriting using sqlite3.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
class ARWDatabase : public RWDatabase {
public:
  // Serializes BindedGNetList object to string.
  static std::string serialize(const BindedGNetList &list);

  // Deserializes BindedGNetList object from string.
  static BindedGNetList deserialize(const std::string &str);

  // Checks whether you can open db that located in path. If you can it creates
  // RWDatabase table in the db. And saves the path as default path to db.
  void linkDB(const std::string &path);

  // Opens connection to db. You must call it before you use ARWDatabase.
  void openDB();

  // Closes connection to db. You must call it after using ARWDatabase.
  void closeDB();

  // Basic interface

  // Find for the key in the local storage and in the database.
  virtual bool find(const TruthTable &key);

  // Get element from the local storage or from the database.
  virtual BindedGNetList get(const TruthTable &key);

  // Database interface.

  // Inserts new value into db.
  void insertIntoDB(const TruthTable &key, const BindedGNetList &value);

  // Update value in db.
  void updateInDB(const TruthTable &key, const BindedGNetList &value);

  // Delete value from db.
  void deleteFromDB(const TruthTable &key);

private:

  static int dummySQLCallback(void *NotUsed, int argc, char **argv, char **azColName) {
    return 0;
  }

  static int selectSQLCallback(void *selectResultPointer,
                               int argc,
                               char **argv,
                               char **azColName) {
    std::string arg1 = std::string(argv[0] == nullptr ? "NULL" : argv[0]);
    std::string arg2 = std::string(argv[1] == nullptr ? "NULL" : argv[1]);
    ((std::vector<std::pair<std::string, std::string> >*)selectResultPointer)
      ->push_back(std::pair<std::string, std::string>(arg1, arg2));
    return 0;
  }

  bool _isLinked = false;
  bool _isOpened = false;
  std::string _pathDB;
  sqlite3 *_db;
  char* _zErrMsg = 0;
  int _rc;
  std::vector<std::pair<std::string, std::string> > _selectResult;
};

} // namespace eda::gate::optimizer
