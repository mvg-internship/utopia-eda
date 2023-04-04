/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */
#include "parserGnet.h"
#include "base/model/signal.h"
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/simulator/simulator.h"
#include "passes/techmap/libparse.h"
#include <kernel/celltypes.h>
#include <kernel/yosys.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace GModel = eda::gate::model;
namespace YLib = Yosys::hashlib;
namespace RTlil = Yosys::RTLIL;

using eda::gate::model::Gate;
using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

void printWires(
    const YLib::dict<RTlil::IdString, RTlil::Wire*> &wires,
    GModel::GNet &net,
    std::map<unsigned, GateId> &inputs,
    std::map<unsigned, GateId> &outputs) {
  for (auto it1 = wires.begin(); it1 != wires.end(); ++it1) {
    unsigned index;
    for (const auto &it2 : RTlil::IdString::global_id_index_) {
      if (it2.second == it1->first.index_) {
        index = it2.second;
      }
    }
    if (it1->second->port_input == 1) {
      GateId inputId = net.addIn();
      inputs.emplace(index, inputId);
    }
    if (it1->second->port_output == 1) {
      GateId outputId;
      outputs.emplace(index,outputId);
    }
  }
}

GateSymbol functionGateSymbol(
    const size_t type,
    std::map<int, std::string> &typeRTLIL,
    const int a) {
  GateSymbol func;
  func = GateSymbol::XXX;
  if (type == ID($_NOT_).index_) {
    func = GateSymbol::NOT;
  }
  if (type == ID($_AND_).index_) {
    func = GateSymbol::AND;
  }
  if (type == ID($_OR_).index_) {
    func = GateSymbol::OR;
  }
  if (type == ID($_XOR_).index_) {
    func = GateSymbol::XOR;
  }
  if (type == ID($_DFF_P_).index_) {
    func = GateSymbol::DFF;
    typeRTLIL.emplace(a, "_DFF_P_");
  }
  if (type == ID($_DFF_N_).index_) {
    func = GateSymbol::DFF;
    typeRTLIL.emplace(a, "_DFF_N_");
  }
  if (type == ID($_DLATCH_P_).index_) {
    func = GateSymbol::LATCH;
    typeRTLIL.emplace(a, "_DLATCH_P_");
  }
  if (type == ID($_DLATCH_N_).index_) {
    func=GateSymbol::LATCH;
    typeRTLIL.emplace(a, "_DLATCH_N_");
  }
  if (type == ID($_DFFSR_PPP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PPP_");
  }
  if (type == ID($_DFFSR_PPN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PPN_");
  }
  if (type == ID($_DFFSR_PNP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PNP_");
  }
  if (type == ID($_DFFSR_PNN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PNN_");
  }
  if (type == ID($_DFFSR_NPP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NPP_");
  }
  if (type == ID($_DFFSR_NPN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NPN_");
  }
  if (type == ID($_DFFSR_NNP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NNP_");
  }
  if (type == ID($_DFFSR_NNN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NNN_");
  }
  return func;
}

Gate::SignalList typeDffsr(
    const std::string type,
    const int node,
    const std::map<int, std::pair<int, int>> &cell,
    const std::map<unsigned, GateId> &inputs) {
  Gate::SignalList inputs_;
  int key;
  for (auto it1: cell) {
    if (it1.second.second == node){
      key=it1.first;
      break;
    }
  }
  auto data = inputs.find(node)->second;
  inputs_.push_back(Gate::Signal::always(data));
  auto clock = inputs.find(cell.find(node)->second.second)->second;
  if (type[7] == 'P') {
    inputs_.push_back(Gate::Signal::posedge(clock));
  } else {
    inputs_.push_back(Gate::Signal::negedge(clock));
  }
  auto reset = inputs.find(cell.find(key)->second.first)->second;
  if (type[8] == 'P') {
    inputs_.push_back(Gate::Signal::level1(reset));
  } else {
    inputs_.push_back(Gate::Signal::level0(reset));
  }
  auto preset = inputs.find(cell.find(key)->first)->second;
  if (type[9] == 'P') {
    inputs_.push_back(Gate::Signal::level1(preset));
  } else {
    inputs_.push_back(Gate::Signal::level0(preset));
  }
  return inputs_;
}

GateId buildNet(
    const int root,
    GModel::GNet &net,
    const std::map<int, GateSymbol> &typeFunc,
    const std::map<int, std::pair<int, int>> &cell,
    const std::map<unsigned, GateId> &inputs,
    const std::map<int, std::string> &typeRTLIL) {
  for (auto it: cell) {
    if (it.second.first == root) {
      if (typeFunc.find(it.first)->second == GateSymbol::XXX) {
        Gate::SignalList inputs1;
        auto leftLeaf = inputs.find(cell.find(root)->second.first)->second;
        inputs1.push_back(Gate::Signal::always(leftLeaf));
        auto rightLeaf = inputs.find(cell.find(root)->second.second)->second;
        inputs1.push_back(Gate::Signal::always(rightLeaf));
        return net.addGate(typeFunc.find(root)->second, inputs1);
      }
      if (typeFunc.find(it.first)->second == GateSymbol::LATCH) {
        if (typeRTLIL.find(it.first)->second == "_DLATCH_P_") {
          auto data = inputs.find(it.first)->second;
          auto clock = inputs.find(it.second.second)->second;
          return net.addLatch(data, clock);
        } else {
          Gate::SignalList inputs1;
          auto data = inputs.find(it.first)->second;
          inputs1.push_back(Gate::Signal::always(data));
          auto clock = inputs.find(it.second.second)->second;
          inputs1.push_back(Gate::Signal::level0(clock));
          return net.addGate(GateSymbol::LATCH, inputs1);
        }
      }
      if (typeFunc.find(it.first)->second == GateSymbol::DFF) {
        if(typeRTLIL.find(it.first)->second == "_DFF_P_") {
          auto data = inputs.find(it.second.second)->second;
          auto clock = inputs.find(it.first)->second;
          return net.addDff(data, clock);
        } else {
          Gate::SignalList inputs1;
          auto data = inputs.find(it.first)->second;
          inputs1.push_back(Gate::Signal::always(data));
          auto clock = inputs.find(it.second.second)->second;
          inputs1.push_back(Gate::Signal::negedge(clock));
          return net.addGate(GateSymbol::DFF, inputs1);
        }
      }
      if (typeFunc.find(it.first)->second == GateSymbol::DFFrs) {
        auto type = typeRTLIL.find(it.first)->second;
        auto inputs1 = typeDffsr(type, it.first, cell, inputs);
        return net.addGate(GateSymbol::DFFrs, inputs1);
      }
    }
  }
  bool flag1 = 0, flag2 = 0;
  if (inputs.find(cell.find(root)->second.first) != inputs.end()) {
    flag1 = 1;
  }
  if (inputs.find(cell.find(root)->second.second) != inputs.end()) {
    flag2 = 1;
  }
  if (flag1 == 1 && flag2 == 1) {
    Gate::SignalList inputs1;
    if (cell.find(root)->second.first != cell.find(root)->second.second) {
      auto leftLeaf = inputs.find(cell.find(root)->second.first)->second;
      inputs1.push_back(Gate::Signal::always(leftLeaf));
      auto rightLeaf = inputs.find(cell.find(root)->second.second)->second;
      inputs1.push_back(Gate::Signal::always(rightLeaf));
    } else {
      auto singleLeaf = inputs.find(cell.find(root)->second.first)->second;
      inputs1.push_back(Gate::Signal::always(singleLeaf));
    }
    return net.addGate(typeFunc.find(root)->second,inputs1);
  }
  if (flag1 == 0 && flag2 == 0) {
    Gate::SignalList inputs1;
    if (cell.find(root)->second.first != cell.find(root)->second.second) {
      auto leftLeaf = cell.find(root)->second.first;
      auto t1 = buildNet(leftLeaf, net, typeFunc, cell, inputs, typeRTLIL);
      inputs1.push_back(Gate::Signal::always(t1));
      auto rightLeaf = cell.find(root)->second.second;
      auto t2 = buildNet(rightLeaf, net, typeFunc, cell, inputs, typeRTLIL);
      inputs1.push_back(Gate::Signal::always(t2));
    } else {
      auto singleLeaf = cell.find(root)->second.first;
      auto t1 = buildNet(singleLeaf, net, typeFunc, cell, inputs, typeRTLIL);
      inputs1.push_back(Gate::Signal::always(t1));
    }
    return net.addGate(typeFunc.find(root)->second, inputs1);
  }
  if (flag1 == 0 && flag2 == 1) {
    Gate::SignalList inputs1;
    auto leftLeaf = cell.find(root)->second.first;
    auto t1 = buildNet(leftLeaf, net, typeFunc, cell, inputs, typeRTLIL);
    inputs1.push_back(Gate::Signal::always(t1));
    auto rightLeaf = inputs.find(cell.find(root)->second.second)->second;
    inputs1.push_back(Gate::Signal::always(rightLeaf));
    return net.addGate(typeFunc.find(root)->second, inputs1);
  }
  if (flag1 == 1 && flag2 == 0) {
    Gate::SignalList inputs1;
    auto leftLeaf = inputs.find(cell.find(root)->second.first)->second;
    inputs1.push_back(Gate::Signal::always(leftLeaf));
    auto rightLeaf = cell.find(root)->second.second;
    auto t2 = buildNet(rightLeaf, net, typeFunc, cell, inputs, typeRTLIL);
    inputs1.push_back(Gate::Signal::always(t2));
    return net.addGate(typeFunc.find(root)->second, inputs1);
  }
  GateId id = 0;
  return id;
}

void printCells(
    const YLib::dict<RTlil::IdString, RTlil::Cell*> &cells,
    GModel::GNet &net,
    std::map<int, GateSymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &typeRTLIL) {
  for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
    bool flag = 0;
    GateSymbol f;
    int a, b, c;
    auto it3 = it1->second->connections_.begin();
    if (it3 != it1->second->connections_.end()) {
      a = it3->second.as_wire()->name.index_;
      f = functionGateSymbol(it1->second->type.index_, typeRTLIL, a);
      typeFunc.emplace(a, f);
      if (f == GateSymbol::NOT) {
        flag = 1;
      }
      ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
      if (!flag) {
        b = it3->second.as_wire()->name.index_;
        flag = 0;
      } else {
        b = it3->second.as_wire()->name.index_;
        c = b;
        cell.emplace(a, std::make_pair(b, c));
      }
      ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
      c = it3->second.as_wire()->name.index_;
      cell.emplace(a, std::make_pair(b, c));
      ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
      a = it3->second.as_wire()->name.index_;
      ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
      b = it3->second.as_wire()->name.index_;
      cell.emplace(c, std::make_pair(a, b));
      f = functionGateSymbol(it1->second->type.index_, typeRTLIL, c);
      typeFunc.emplace(c, f);
    }
  }
}

void printConnections(
    const std::vector<std::pair<RTlil::SigSpec, RTlil::SigSpec>> &connections,
    GModel::GNet &net,
    std::map<int, GateSymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<unsigned, GateId> &inputs,
    std::map<unsigned, GateId> &outputs,
    std::map<int, std::string> typeRTLIL) {
  int root;
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    if (inputs.find(it1->second.as_wire()->name.index_) == inputs.end()) {
      root = it1->second.as_wire()->name.index_;
      auto output = buildNet(root, net, typeFunc, cell, inputs, typeRTLIL);
      outputs.find(it1->first.as_wire()->name.index_)->second = net.addOut(output);
    } else {
      Gate::SignalList inputs1;
      auto t1 = inputs.find(it1->second.as_wire()->name.index_)->second;
      inputs1.push_back(Gate::Signal::always(t1));
      auto output = net.addGate(GateSymbol::NOP, inputs1);
      outputs.find(it1->first.as_wire()->name.index_)->second = net.addOut(output);
    }
  }
}

void translateModuleToGNet(
    const std::pair<RTlil::IdString, RTlil::Module*> &m,
    GModel::GNet &net) {
  std::map<unsigned, GateId> outputs;
  std::map<unsigned, GateId> inputs;
  std::map<int, std::pair<int, int>> cell;
  std::map<int, GateSymbol> typeFunc;
  std::map<int, std::string> typeRTLIL;
  printWires(m.second->wires_, net, inputs, outputs);
  printCells(m.second->cells_, net, typeFunc, cell, typeRTLIL);
  printConnections(m.second->connections_, net, typeFunc, cell, inputs, outputs, typeRTLIL);
}

void translateDesignToGNet(const RTlil::Design &des, std::vector<GModel::GNet> &vec) {
  for (auto &m : des.modules_) {
    GModel::GNet net(0);
    translateModuleToGNet(m, net);
    vec.push_back(net);
  }
}

