/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */

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

void printModules(const int ind, std::ostream &out) {
  for (const auto &it1 : yosys::IdString::global_id_index_) {
    if (it1.second == ind) {
      out << it1.first << " - module of index: " << it1.second << "\n";
    }
  }
}
void printWires(
    const lib::dict<yosys::IdString, yosys::Wire*> &wires,
    std::ostream &out,
    gmodel::GNet &net,
    std::map<unsigned, gid> &inputs,
    std::map<unsigned, gid> &outputs) {
  out << "Wires:" << "\n";
  for (auto it1=wires.begin(); it1 != wires.end(); ++it1) {
    unsigned index;
    for(const auto &it2 : yosys::IdString::global_id_index_) {
      if (it2.second == it1->first.index_) {
        out << "  " << it2.first << " - wire of index: " << it2.second << "\n";
        index=it2.second;
      }
    }
    if (it1->second->width > 1) {
      out<< "    type: bus with width " << it1->second->width << "\n";
    } else {
      out << "    type: wire" << "\n";
    }
    out << "    start_offset: " << it1->second->start_offset << "\n";
    out << "    port_id: " << it1->second->port_id << "\n";
    out << "    port_input: " << it1->second->port_input << "\n";
    if (it1->second->port_input == 1) {
      gid inputId = net.addIn();
      inputs.emplace(index, inputId);
    }
    out << "    port_output: " << it1->second->port_output << "\n";
    if (it1->second->port_output == 1) {
      gid outputId;
      outputs.emplace(index,outputId);
    }
    out << "    upto: " << it1->second->upto << "\n";
    out << "\n";
  }
}
gsymbol functionGateSymbol(const size_t type, std::map<int, std::string> &typeRTLIL, const int a) {
  gsymbol func;
  func=gsymbol::XXX;
  std::cout <<" functionGateSymbol " << ID($_DFFSR_NNN_).index_ << "the type of the func\n";
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
      std::cout <<" Build Net" <<ID($_DFFSR_NNN_).index_<<"the type of the func\n";

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
    std::ostream &out,
    gmodel::GNet &net,
    std::map<int, gsymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &typeRTLIL) {
  out << "Cells:" << "\n";
  for (auto it1=cells.begin(); it1 != cells.end(); ++it1) {
    for (const auto &it2 : yosys::IdString::global_id_index_) {
      if (it2.second == it1->first.index_){
        out << "  " << it2.first << " cell of index " << it2.second << " type " << it1->second->type.index_ << "\n";
      }
    }
    for (auto it3 = it1->second->connections_.begin(); it3 != it1->second->connections_.end(); ++it3) {
      out << "    Connections: " << it3->first.index_ << "\n";
      out << "      name: " << it3->second.as_wire()->name.index_ << "\n";
    }
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
    std::ostream &out,
    gmodel::GNet &net,
    std::map<int, gsymbol> &typeFunc,
    std::map<int, std::pair<int, int>> &cell,
    std::map<unsigned, gid> &inputs,
    std::map<unsigned, gid> &outputs,
    std::map<int, std::string> typeRTLIL) {
  out << "Connections count: " << connections.size() << "\n";
  int root;
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    out<<"    Wire " << it1->first.as_wire()->name.index_ << " connects with " << it1->second.as_wire()->name.index_ << "\n";
    if(inputs.find(it1->second.as_wire()->name.index_) == inputs.end()) {
      root=it1->second.as_wire()->name.index_;
      auto output=buildNet(root, net, typeFunc, cell, inputs, typeRTLIL);
      outputs.find(it1->first.as_wire()->name.index_)->second=net.addOut(output);
      std::cout << net;
    } else {
      gate::SignalList inputs1;
      inputs1.push_back(gate::Signal::always(inputs.find(it1->second.as_wire()->name.index_)->second));
      auto output=net.addGate(gsymbol::NOP, inputs1);
      outputs.find(it1->first.as_wire()->name.index_)->second=net.addOut(output);
      std::cout << net;
    }
  }
}
void printMemory(const lib::dict<yosys::IdString, yosys::Memory*> &memories, std::ostream &out) {
  out << "Memory count: " << memories.size() << "\n";
  for (auto it1 = memories.begin(); it1 != memories.end(); ++it1){
    out << "  memory " << it1->second << "\n";
    out << "  name: " << it1->second->name.index_ << "\n";
    out << "  width: " << it1->second->width << "\n";
    out << "  start_offset: " << it1->second->start_offset << "\n";
    out << "  size: " << it1->second->size << "\n";
    out << "\n";
  }
}
void printActions(const std::vector<yosys::SigSig> &actions, std::ostream &out) {
  for (auto &it : actions){
    out << "    " << (*it.first.as_chunk().wire).name.index_ << "\n";
  }
}
void printSyncs(const std::vector<yosys::SyncRule*> &syncs, std::ostream &out) {
  out << "  syncs\n";
  for (auto it1=syncs.begin(); it1 != syncs.end(); ++it1){
    out << "    type " << (*it1)->type << "\n";
    out << "  signal\n";
    out << "    size " << (*it1)->signal.size() << "\n";
    out << "    as_wire index " << (*it1)->signal.as_wire()->name.index_ << "\n";
    out << "  actions\n";
    printActions((*it1)->actions, out);
  }
}
void printProcesses(const lib::dict<yosys::IdString, yosys::Process*> &processes, std::ostream &out) {
  out << "Processes count: " << processes.size() << "\n";
  for (auto it1 = processes.begin(); it1 != processes.end(); ++it1){
    out << "  name " << it1->second->name.index_ << "\n";
    printSyncs(it1->second->syncs, out);
    out << "\n";
  }
}
void printPorts(const std::vector<yosys::IdString> &ports, std::ostream &out) {
  out << "Ports count: " << ports.size() << "\n";
  for (auto &it1 : ports){
    out << "  port " << it1.index_ << "\n";
  }
}
void translateModuleToGNet(
    const std::pair<yosys::IdString, yosys::Module*> &m,
    std::ostream &out,
    gmodel::GNet &net) {
  std::map<unsigned, gid> outputs;
  std::map<unsigned, gid> inputs;
  std::map<int, std::pair<int, int>> cell;
  std::map<int, gsymbol> typeFunc;
  std::map<int, std::string> typeRTLIL;
  printModules(m.first.index_, out);
  out << "\n";
  printWires(m.second->wires_, out, net, inputs, outputs);
  out << "\n";
  printCells(m.second->cells_, out, net, typeFunc, cell, typeRTLIL);
  out << "\n" ;
  printConnections(m.second->connections_, out, net, typeFunc, cell, inputs, outputs, typeRTLIL);
  out << "\n";
  printMemory(m.second->memories, out);
  out << "\n";
  printProcesses(m.second->processes, out);
  out << "\n";
  printPorts(m.second->ports, out);
  out << "\n";
  out << "Refcount wires count: " << m.second->refcount_wires_ << "\n";
  out << "Refcount cells count: " << m.second->refcount_cells_ << "\n";
  out << "Monitors count: " << m.second->monitors.size() << "\n";
  out << "Avail_parameters count: " << m.second->avail_parameters.size() << "\n";
  out << "\n";
}
void translateDesignToGNet(const yosys::Design &des, std::ostream &out, std::vector<gmodel::GNet> &vec) {
  for (auto &m : des.modules_) {
    gmodel::GNet net(0);
    translateModuleToGNet(m, out, net);
    vec.push_back(net);
  }
}
int main(int argc, char* argv[]) {
  std::ostream& out = std::cout;
  Yosys::yosys_setup();
  std::vector<gmodel::GNet> vec;
  for (size_t o=1;o<argc;++o) {
    yosys::Design design;
    Yosys::run_frontend(argv[o], "liberty", &design, nullptr);
    translateDesignToGNet(design, out, vec);
  }
  for (auto i : vec) {
    std::cout << i << "\n";
  }
  Yosys::yosys_shutdown();
}
