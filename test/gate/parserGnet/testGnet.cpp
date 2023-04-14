#include "gate/parserGnet/parserGnet.h"
#include <iostream>

namespace GModel = eda::gate::model;

using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

GModel::GNet createBuf() {
  GModel::GNet net(0);
  GateId a = net.addIn();
  GateId out = net.addGate(GateSymbol::NOP, a);
  net.addOut(out);
  net.sortTopologically();
  return net;
}

GModel::GNet createNot() {
  GModel::GNet net(0);
  GateId a = net.addIn();
  GateId out = net.addGate(GateSymbol::NOT, a);
  net.addOut(out);
  net.sortTopologically();
  return net;
}

GModel::GNet createXor() {
  GModel::GNet net(0);
  GateId a = net.addIn();
  GateId b = net.addIn();
  GateId out = net.addGate(GateSymbol::XOR, a, b);
  net.addOut(out);
  net.sortTopologically();
  return net;
}

GModel::GNet createOr() {
  GModel::GNet net(0);
  GateId a = net.addIn();
  GateId b = net.addIn();
  GateId out = net.addGate(GateSymbol::OR, a, b);
  net.addOut(out);
  net.sortTopologically();
  return net;
}

GModel::GNet createAnd() {
  GModel::GNet net(0);
  GateId a = net.addIn();
  GateId b = net.addIn();
  GateId out = net.addGate(GateSymbol::AND, a, b);
  net.addOut(out);
  net.sortTopologically();
  return net;
}

void fillSource(NetData &source) {
  source.combNets.push_back(createBuf());
  source.combNets.push_back(createNot());
  source.combNets.push_back(createXor());
  source.combNets.push_back(createOr());
  source.combNets.push_back(createAnd());
}

void fillVectorMean(
    const std::map<GModel::GNet*, std::vector<uint64_t>> &answ,
    std::vector<uint64_t> &mean) {
  for (auto it : answ) {
    for (auto it1 : it.second) {
      mean.push_back(it1);
    }
  }
}

int main() {
  NetData vec;
  NetData source;
  fillSource(source);
  translateLibertyToDesign("normal1.lib", vec);
  auto answ0 = truthTab(source);
  auto answ1 = truthTab(vec);
  std::vector<uint64_t> mean0;
  std::vector<uint64_t> mean1;
  fillVectorMean(answ0, mean0);
  fillVectorMean(answ1, mean1);
  if (mean0 == mean1) {
    std::cout << "matched\n";
  }
}
