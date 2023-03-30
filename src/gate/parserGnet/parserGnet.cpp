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

namespace gmodel = eda::gate::model;
namespace lib = Yosys::hashlib;
namespace yosys = Yosys::RTLIL;

using gate = gmodel::Gate;
using gsymbol = gmodel::GateSymbol;
using gid = gmodel::Gate::Id;

void printWires(
    const lib::dict<yosys::IdString, yosys::Wire*> &wires,
    gmodel::GNet &net,
    std::map<unsigned, gid> &inputs,
    std::map<unsigned, gid> &outputs) {
  for (auto it1=wires.begin(); it1 != wires.end(); ++it1) {
    unsigned index;
    for(const auto &it2 : yosys::IdString::global_id_index_) {
      if (it2.second == it1->first.index_) {
        index=it2.second;
      }
    }
    if (it1->second->port_input == 1) {
      gid inputId = net.addIn();
      inputs.emplace(index, inputId);
    }
    if (it1->second->port_output == 1) {
      gid outputId;
      outputs.emplace(index,outputId);
    }
  }
}
gsymbol functionGateSymbol(const size_t type, std::map<int, std::string> &typeRTLIL, const int a) {
  gsymbol func;
  func=gsymbol::XXX;
  if (type == ID($_NOT_).index_){
    func=gsymbol::NOT;
  }
  if (type == ID($_AND_).index_){
    func=gsymbol::AND;
  }
  if (type == ID($_OR_).index_){
    func=gsymbol::OR;
  }
  if (type == ID($_XOR_).index_){
    func=gsymbol::XOR;
  }
  if (type == ID($_DFF_P_).index_){
    func=gsymbol::DFF;
    typeRTLIL.emplace(a, "_DFF_P_");
  }
  if (type == ID($_DFF_N_).index_){
    func=gsymbol::DFF;
    typeRTLIL.emplace(a, "_DFF_N_");
  }
  if (type == ID($_DLATCH_P_).index_){
    func=gsymbol::LATCH;
    typeRTLIL.emplace(a, "_DLATCH_P_");
  }
  if (type == ID($_DLATCH_N_).index_){
    func=gsymbol::LATCH;
    typeRTLIL.emplace(a, "_DLATCH_N_");
  }
  if (type == ID($_DFFSR_PPP_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PPP_");
  }
  if (type == ID($_DFFSR_PPN_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PPN_");
  }
  if (type == ID($_DFFSR_PNP_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PNP_");
  }
  if (type == ID($_DFFSR_PNN_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_PNN_");
  }
  if (type == ID($_DFFSR_NPP_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NPP_");
  }
  if (type == ID($_DFFSR_NPN_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NPN_");
  }
  if (type == ID($_DFFSR_NNP_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NNP_");
  }
  if (type == ID($_DFFSR_NNN_).index_){
    func=gsymbol::DFFrs;
    typeRTLIL.emplace(a, "_DFFSR_NNN_");
  }
  return func;
}
gate::SignalList typeDffsr(
    const std::string type,
    const int node,
    const std::map<int, std::pair<int, int>> &cell,
    const std::map<unsigned, gid> &inputs) {
  gate::SignalList inputs_;
  int key;
  for(auto it1: cell) {
    if (it1.second.second == node){
      key=it1.first;
      break;
    }
  }
  inputs_.push_back(gate::Signal::always(inputs.find(node)->second));
  if (type[7] == 'P') {
    inputs_.push_back(gate::Signal::posedge(inputs.find(cell.find(node)->second.second)->second));
  } else {
    inputs_.push_back(gate::Signal::negedge(inputs.find(cell.find(node)->second.second)->second));
  }
  if(type[8] == 'P') {
    inputs_.push_back(gate::Signal::level1(inputs.find(cell.find(key)->second.first)->second));
  } else {
    inputs_.push_back(gate::Signal::level0(inputs.find(cell.find(key)->second.first)->second));
  }
  if(type[9] == 'P') {
    inputs_.push_back(gate::Signal::level1(inputs.find(cell.find(key)->first)->second));
  } else {
    inputs_.push_back(gate::Signal::level0(inputs.find(cell.find(key)->first)->second));
  }
  return inputs_;
}
gid buildNet(
    const int root,
    gmodel::GNet &net,
    const std::map<int, gsymbol> &typeFunc,
    const std::map<int, std::pair<int, int>> &cell,
    const std::map<unsigned, gid> &inputs,
    const std::map<int, std::string> &typeRTLIL) {
  for (auto it: cell) {
    if (it.second.first == root) {
      if (typeFunc.find(it.first)->second == gsymbol::XXX) {
        gate::SignalList inputs1;
        inputs1.push_back(gate::Signal::always(inputs.find(cell.find(root)->second.first)->second));
        inputs1.push_back(gate::Signal::always(inputs.find(cell.find(root)->second.second)->second));
        return net.addGate(typeFunc.find(root)->second,inputs1);
      }
      if (typeFunc.find(it.first)->second == gsymbol::LATCH) {
        if(typeRTLIL.find(it.first)->second == "_DLATCH_P_") {
          return net.addLatch(inputs.find(it.first)->second, inputs.find(it.second.second)->second);
        } else {
          return net.addGate(gsymbol::LATCH, {gate::Signal::always(inputs.find(it.first)->second), gate::Signal::level0(inputs.find(it.second.second)->second)});
        }
      }
      if (typeFunc.find(it.first)->second == gsymbol::DFF) {
        if(typeRTLIL.find(it.first)->second == "_DFF_P_") {
          return net.addDff(inputs.find(it.second.second)->second,inputs.find(it.first)->second);
        } else {
          return net.addGate(gsymbol::DFF, {gate::Signal::always(inputs.find(it.first)->second), gate::Signal::negedge(inputs.find(it.second.second)->second)});
        }
      }
      if(typeFunc.find(it.first)->second == gsymbol::DFFrs) {
        return net.addGate(gsymbol::DFFrs, typeDffsr(typeRTLIL.find(it.first)->second, it.first, cell, inputs));
      }
    }
  }
  bool flag1=0, flag2=0;
  if(inputs.find(cell.find(root)->second.first) != inputs.end()) {
    flag1=1;
  }
  if(inputs.find(cell.find(root)->second.second) != inputs.end()) {
    flag2=1;
  }
  if (flag1 == 1 && flag2 == 1) {
    gate::SignalList inputs1;
    if (cell.find(root)->second.first!=cell.find(root)->second.second) {
      inputs1.push_back(gate::Signal::always(inputs.find(cell.find(root)->second.first)->second));
      inputs1.push_back(gate::Signal::always(inputs.find(cell.find(root)->second.second)->second));
    } else {
      inputs1.push_back(gate::Signal::always(inputs.find(cell.find(root)->second.first)->second));
    }
    return net.addGate(typeFunc.find(root)->second,inputs1);
  }
  if (flag1 == 0 && flag2 == 0) {
    gate::SignalList inputs1;
    if (cell.find(root)->second.first != cell.find(root)->second.second) {
      inputs1.push_back(gate::Signal::always(buildNet(cell.find(root)->second.first, net, typeFunc, cell, inputs, typeRTLIL)));
      inputs1.push_back(gate::Signal::always(buildNet(cell.find(root)->second.second, net, typeFunc, cell, inputs, typeRTLIL)));
    } else {
      inputs1.push_back(gate::Signal::always(buildNet(cell.find(root)->second.first, net, typeFunc, cell, inputs, typeRTLIL)));
    }
    return net.addGate(typeFunc.find(root)->second,inputs1);
  }
  if (flag1 == 0 && flag2 == 1) {
    gate::SignalList inputs1;
    inputs1.push_back(gate::Signal::always(buildNet(cell.find(root)->second.first, net, typeFunc, cell, inputs, typeRTLIL)));
    inputs1.push_back(gate::Signal::always(inputs.find(cell.find(root)->second.second)->second));
    return net.addGate(typeFunc.find(root)->second,inputs1);
  }
  if (flag1 == 1 && flag2 == 0) {
    gate::SignalList inputs1;
    inputs1.push_back(gate::Signal::always(inputs.find(cell.find(root)->second.first)->second));
    inputs1.push_back(gate::Signal::always(buildNet(cell.find(root)->second.second, net, typeFunc, cell, inputs, typeRTLIL)));
    return net.addGate(typeFunc.find(root)->second,inputs1);
  }
}
void printCells(
    const lib::dict<yosys::IdString, yosys::Cell*> &cells,
    gmodel::GNet &net,
    std::map<int, gsymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &typeRTLIL) {
  for (auto it1=cells.begin(); it1 != cells.end(); ++it1) {
    bool flag=0;
    gsymbol f;
    int a, b, c;
    auto it3=it1->second->connections_.begin();
    if (it3 != it1->second->connections_.end()) {
      a=it3->second.as_wire()->name.index_;
      f=functionGateSymbol(it1->second->type.index_, typeRTLIL, a);
      typeFunc.emplace(a, f);
      if (f == gsymbol::NOT) {
        flag=1;
      }
       ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
      if (!flag) {
        b=it3->second.as_wire()->name.index_;
        flag=0;
      } else {
        b=it3->second.as_wire()->name.index_;
        c=b;
        cell.emplace(a, std::make_pair(b, c));
      }
      ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
      c=it3->second.as_wire()->name.index_;
      cell.emplace(a, std::make_pair(b, c));
      ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
       a=it3->second.as_wire()->name.index_;
       ++it3;
    }
    if (it3 != it1->second->connections_.end()) {
      b=it3->second.as_wire()->name.index_;
      cell.emplace(c, std::make_pair(a, b));
      f=functionGateSymbol(it1->second->type.index_, typeRTLIL, c);
      typeFunc.emplace(c, f);
    }
  }
}
void printConnections(
    const std::vector<std::pair<yosys::SigSpec, yosys::SigSpec>> &connections,
    gmodel::GNet &net,
    std::map<int, gsymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<unsigned, gid> &inputs,
    std::map<unsigned, gid> &outputs,
    std::map<int, std::string> typeRTLIL) {
  int root;
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    if(inputs.find(it1->second.as_wire()->name.index_) == inputs.end()) {
      root=it1->second.as_wire()->name.index_;
      auto output=buildNet(root, net, typeFunc, cell, inputs, typeRTLIL);
      outputs.find(it1->first.as_wire()->name.index_)->second=net.addOut(output);
    } else {
      gate::SignalList inputs1;
      inputs1.push_back(gate::Signal::always(inputs.find(it1->second.as_wire()->name.index_)->second));
      auto output=net.addGate(gsymbol::NOP, inputs1);
      outputs.find(it1->first.as_wire()->name.index_)->second=net.addOut(output);
    }
  }
}
void translateModuleToGNet(
    const std::pair<yosys::IdString, yosys::Module*> &m,
    gmodel::GNet &net) {
  std::map<unsigned, gid> outputs;
  std::map<unsigned, gid> inputs;
  std::map<int, std::pair<int, int>> cell;
  std::map<int, gsymbol> typeFunc;
  std::map<int, std::string> typeRTLIL;
  printWires(m.second->wires_, net, inputs, outputs);
  printCells(m.second->cells_, net, typeFunc, cell, typeRTLIL);
  printConnections(m.second->connections_, net, typeFunc, cell, inputs, outputs, typeRTLIL);
}
void translateDesignToGNet(const yosys::Design &des, std::vector<gmodel::GNet> &vec) {
  for (auto &m : des.modules_) {
    gmodel::GNet net(0);
    translateModuleToGNet(m, net);
    vec.push_back(net);
  }
}

