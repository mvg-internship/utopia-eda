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

#include "sqlite3-bind.h"
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

    // "Virtual" input gate ID.
    using InputId = uint32_t;
    // Contain value vector of boolean function.
    // Represents boolean function of 6 variables.
    using TruthTable = uint64_t;

    // Binds "virtual" input IDs to real primary input IDs of GNet.
    using GateBindings = std::unordered_map<InputId, Gate::Id>;
    using ReversedGateBindings = std::unordered_map<Gate::Id, InputId>;

    using InputIdDoubleMap = std::unordered_map<InputId, double>;

    struct BoundGNet {
      std::shared_ptr<GNet> net;
      GateBindings bindings;
      InputIdDoubleMap inputsDelay;
    };

    using BoundGNetList = std::vector<BoundGNet>;

    // Basic interface.
    virtual bool contains(const TruthTable &key) {
      return (_storage.find(key) != _storage.end());
    }

    virtual BoundGNetList get(const TruthTable &key) {
      if (!contains(key)) {
        return BoundGNetList();
      }
      return _storage[key];
    }

    virtual void set(const TruthTable &key, const BoundGNetList &value) {
      _storage[key] = value;
    }

    virtual void erase(const TruthTable &key) {
      _storage.erase(key);
    }

    virtual bool empty() {
      return _storage.empty();
    }

    virtual ~RWDatabase() { }

  protected:
    std::unordered_map<TruthTable, BoundGNetList> _storage;

  };

/**
* \brief Implements storage that contains GNets for rewriting using sqlite3.
* \author <a href="mailto:mrpepelulka@gmail.com">Rustamkhan Ramaldanov</a>
*/
  class SQLiteRWDatabase : public RWDatabase {
  public:
    // Serializes BoundGNetList object to string.
    static std::string serialize(const BoundGNetList &list);

    // Deserializes BoundGNetList object from string.
    static BoundGNetList deserialize(const std::string &str);

    // Checks whether you can open DB that located in path. If you can it creates
    // RWDatabase table in the DB. And saves the path as default path to DB.
    void linkDB(const std::string &path);

    // Opens connection to DB. You must call it before you use SQLiteRWDatabase.
    void openDB();

    // Closes connection to DB. You must call it after using SQLiteRWDatabase.
    void closeDB();

    // Basic interface

    // Find for the key in the local storage and in the database.
    virtual bool contains(const TruthTable &key);

    // Get element from the local storage or from the database.
    virtual BoundGNetList get(const TruthTable &key);

    // Database interface.

    // Inserts new value into DB.
    void insertIntoDB(const TruthTable &key, const BoundGNetList &value);

    // Update value in DB.
    void updateInDB(const TruthTable &key, const BoundGNetList &value);

    // Delete value from DB.
    void deleteFromDB(const TruthTable &key);

  private:

    bool dbContainsRWTable();

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

    std::string _dbTableName = "RWDatabase";
    std::string _dbKeyName = "TruthTable";
    std::string _dbValueName = "BGNetList";
    std::string _dbKeyType = "BIGINT";
    std::string _dbValueType = "TEXT";

    bool _isLinked = false;
    bool _isOpened = false;
    std::string _pathDB;
    sqlite3 *_db;
    char* _zErrMsg = 0;
    int _rc;
    std::vector<std::pair<std::string, std::string> > _selectResult;
  };

} // namespace eda::gate::optimizer