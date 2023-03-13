#include <iostream>

#include "gate/optimizer/rwdatabase.h"

using namespace eda::gate::optimizer;
using namespace eda::gate::model;

using Gate = eda::gate::model::Gate;
using GateList = std::vector<Gate::Id>;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;

using RWDatabase = eda::gate::optimizer::RWDatabase;
using ARWDatabase = eda::gate::optimizer::ARWDatabase;

static std::unique_ptr<GNet> makeAnd2(Gate::SignalList
                                      &inputs,
                                      Gate::Id &outputId,
                                      GateList &varList) {
  auto net = std::make_unique<GNet>();

  Gate::Id x0Id = net->newGate(), x1Id = net->newGate();
  Gate::SignalList and0Inputs = {Gate::Signal::always(x0Id),
                                 Gate::Signal::always(x1Id)};

  inputs = and0Inputs;
  outputId = net->addGate(GateSymbol::AND, and0Inputs);
  varList = {x0Id, x1Id};

  net->sortTopologically();

  return net;
}

void test(ARWDatabase &arw) {
  Gate::SignalList inputs; Gate::Id outputId; GateList varList;
  std::shared_ptr<GNet> dummy = std::make_shared<GNet>(*makeAnd2(inputs, outputId, varList));
  RWDatabase::GateBindings bindings = {{0, inputs[0].node()}, {1, inputs[1].node()}};
  RWDatabase::ValueVector valueVector = 1;

  dummy->sortTopologically();

  RWDatabase::BindedGNetList bgl = {{dummy, bindings}};

  arw.insertIntoDB(valueVector, bgl);

  auto newBgl = arw.get(valueVector);

  std::cout << *(newBgl[0].net) << '\n';
}

void test2(ARWDatabase &arw) {
  arw.deleteFromDB(1);
}

int main() {
  ARWDatabase db;

  db.linkDB("test.db");
  db.openDB();

  test(db);
  test2(db);
  test(db);
  test2(db);

  db.closeDB();
  remove("test.db");
}