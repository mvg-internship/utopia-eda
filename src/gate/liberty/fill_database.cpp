#include "gate/truth_table/truth_table.h"
#include "fill_database.h"

using eda::gate::optimizer::RWDatabase;
using eda::gate::optimizer::SQLiteRWDatabase;

void fillDatabase(
    NetData &nets,
    SQLiteRWDatabase &database) {
  std::uint32_t id = 0;
  for (auto& net: nets.combNets) {
    RWDatabase::BoundGNet bounder;
    for (auto link: net->sourceLinks()) {
      bounder.bindings.emplace(id++, link.target);
    }
    auto key = buildTruthTab(net.get())[0];
    bounder.net.reset(net.release());
    RWDatabase::BoundGNetList vectorBounder;
    vectorBounder.push_back(bounder);
    database.insertIntoDB(key, vectorBounder);
  }
}
