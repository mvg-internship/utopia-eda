/**
 * \brief Implements translation from Liberty format file to GNet.
 * \author <a href="mailto:anushakov@edu.hse.ru">Aleksander Ushakov</a>
 */
#include "parserGnet.h"
#include <map>
#include <string>

namespace GModel = eda::gate::model;
namespace YLib = Yosys::hashlib;
namespace RTlil = Yosys::RTLIL;

using eda::gate::model::Gate;
using eda::gate::model::GateSymbol;
using GateId = eda::gate::model::Gate::Id;

/// The functions creates inputs and outputs containers.
static void printWires(
    const YLib::dict<RTlil::IdString, RTlil::Wire*> &wires,
    GModel::GNet &net,
    std::map<size_t, GateId> &inputs,
    std::map<size_t, GateId> &outputs) {
  for (auto it1 = wires.begin(); it1 != wires.end(); ++it1) {
    size_t index;
    auto wireIdStringYosys = it1->first;
    index = wireIdStringYosys.index_;
    auto propertyWireYosys = it1->second;
    if (propertyWireYosys->port_input == 1) {
      GateId inputId = net.addIn();
      inputs.emplace(index, inputId);
    }
    if (propertyWireYosys->port_output == 1) {
      GateId outputId;
      outputs.emplace(index, outputId);
    }
  }
}

/// The function determinates GateSymbol.
static GateSymbol functionGateSymbol(
    const size_t type,
    std::map<size_t, std::string> &typeRTLIL,
    const size_t index) {
  GateSymbol func;
  func = GateSymbol::XXX;
  // ID(...) is a macros of RTLIL Design.
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
  // Filling typeRTLIL, if it is specific cell.
  if (type == ID($_DFF_P_).index_) {
    func = GateSymbol::DFF;
    typeRTLIL.emplace(index, "_DFF_P_");
  }
  if (type == ID($_DFF_N_).index_) {
    func = GateSymbol::DFF;
    typeRTLIL.emplace(index, "_DFF_N_");
  }
  if (type == ID($_DLATCH_P_).index_) {
    func = GateSymbol::LATCH;
    typeRTLIL.emplace(index, "_DLATCH_P_");
  }
  if (type == ID($_DLATCH_N_).index_) {
    func=GateSymbol::LATCH;
    typeRTLIL.emplace(index, "_DLATCH_N_");
  }
  if (type == ID($_DFFSR_PPP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_PPP_");
  }
  if (type == ID($_DFFSR_PPN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_PPN_");
  }
  if (type == ID($_DFFSR_PNP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_PNP_");
  }
  if (type == ID($_DFFSR_PNN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_PNN_");
  }
  if (type == ID($_DFFSR_NPP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_NPP_");
  }
  if (type == ID($_DFFSR_NPN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_NPN_");
  }
  if (type == ID($_DFFSR_NNP_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_NNP_");
  }
  if (type == ID($_DFFSR_NNN_).index_) {
    func = GateSymbol::DFFrs;
    typeRTLIL.emplace(index, "_DFFSR_NNN_");
  }
  return func;
}

/// The function creates input list for creating DFFsr module.
static Gate::SignalList typeDffsr(
    const std::string type,
    const size_t upperField,
    const std::map<size_t, std::pair<size_t, size_t>> &cell,
    const std::map<size_t, GateId> &inputs) {
  Gate::SignalList inputs_;
  size_t secondUpperField;
  // Find second cell, contains preset and reset pins.
  for (auto it1: cell) {
    size_t secondLowField = it1.second.second;
    if (secondLowField == upperField){
      secondUpperField=it1.first;
      break;
    }
  }
  // uppearField is root of second cell and index of data pin same time.
  GateId data = inputs.find(upperField)->second;
  inputs_.push_back(Gate::Signal::always(data));
  // lowestField is index of clock pin, which places in first cell.
  size_t lowestField = cell.find(upperField)->second.second;
  GateId clock = inputs.find(lowestField)->second;
  // type[7] is P - positive or N - negative clock pin, where 'type' consist of information about the cell.
  if (type[7] == 'P') {
    inputs_.push_back(Gate::Signal::posedge(clock));
  } else {
    inputs_.push_back(Gate::Signal::negedge(clock));
  }
  // secondMidField is index of reset pin, which places in second cell
  size_t secondMidField = cell.find(secondUpperField)->second.first;
  GateId reset = inputs.find(secondMidField)->second;
  //type[8] is P - positive or N - negative reset pin, where 'type' consist of information about the cell.
  if (type[8] == 'P') {
    inputs_.push_back(Gate::Signal::level1(reset));
  } else {
    inputs_.push_back(Gate::Signal::level0(reset));
  }
  // type[9] is P - positive or N - negative preset pin, where 'type' consist of information about the cell.
  GateId preset = inputs.find(secondUpperField)->second;
  if (type[9] == 'P') {
    inputs_.push_back(Gate::Signal::level1(preset));
  } else {
    inputs_.push_back(Gate::Signal::level0(preset));
  }
  return inputs_;
}

/// The function creates Net with help a root and a tree.
static GateId buildNet(
    const size_t root,
    GModel::GNet &net,
    const std::map<size_t, GateSymbol> &typeFunc,
    const std::map<size_t, std::pair<size_t, size_t>> &cell,
    const std::map<size_t, GateId> &inputs,
    const std::map<size_t, std::string> &typeRTLIL) {
  // The search specific cell: Latch, Dff, Dffsr.
  for (auto it: cell) {
    size_t middleField = it.second.first;
    if (middleField == root) {
      Gate::SignalList inputs_;
      size_t upperField = it.first;
      size_t lowerField = it.second.second;
      GateSymbol curr = typeFunc.find(upperField)->second;
      // The checking that middleField is a wire of specific cell.
      if (curr == GateSymbol::LATCH) {
        GateId data = inputs.find(upperField)->second;
        GateId clock = inputs.find(lowerField)->second;
        if (typeRTLIL.find(upperField)->second == "_DLATCH_P_") {
          return net.addLatch(data, clock);
        } else {          
          inputs_.push_back(Gate::Signal::always(data));
          inputs_.push_back(Gate::Signal::level0(clock));
          return net.addGate(GateSymbol::LATCH, inputs_);
        }
      }
      if (curr == GateSymbol::DFF) {
        GateId data = inputs.find(lowerField)->second;
        GateId clock = inputs.find(upperField)->second;
        if (typeRTLIL.find(upperField)->second == "_DFF_P_") {
          return net.addDff(data, clock);
        } else {
          inputs_.push_back(Gate::Signal::always(data));
          inputs_.push_back(Gate::Signal::negedge(clock));
          return net.addGate(GateSymbol::DFF, inputs_);
        }
      }
      if (curr == GateSymbol::DFFrs) {
        std::string type = typeRTLIL.find(upperField)->second;
        inputs_ = typeDffsr(type, upperField, cell, inputs);
        return net.addGate(GateSymbol::DFFrs, inputs_);
      }
    }
  }
  bool flag1 = 0, flag2 = 0;
  size_t leftLeaf = cell.find(root)->second.first;
  size_t rightLeaf = cell.find(root)->second.second;
  Gate::SignalList inputs_;
  // If leaf is a wire of pin type, then flag == 1, else it is just wire.
  if (inputs.find(leftLeaf) != inputs.end()) {
    flag1 = 1;
  }
  if (inputs.find(rightLeaf) != inputs.end()) {
    flag2 = 1;
  }
  GateSymbol func = typeFunc.find(root)->second;
  if (flag1 && flag2) {
    // The condition is needed to check neg operator for the cell.
    // leftLeaf == rightLeaf, when the cell contains of neg operator.
    if (leftLeaf != rightLeaf) {
      GateId leftLeaf_ = inputs.find(leftLeaf)->second;
      inputs_.push_back(Gate::Signal::always(leftLeaf_));
      GateId rightLeaf_ = inputs.find(rightLeaf)->second;
      inputs_.push_back(Gate::Signal::always(rightLeaf_));
    } else {
      GateId oneLeaf = inputs.find(leftLeaf)->second;
      inputs_.push_back(Gate::Signal::always(oneLeaf));
    }
    return net.addGate(func, inputs_);
  }
  if (flag1 && !flag2) {
    GateId leftLeaf_ = inputs.find(leftLeaf)->second;
    inputs_.push_back(Gate::Signal::always(leftLeaf_));
    GateId rightLeaf_ = buildNet(rightLeaf, net, typeFunc, cell, inputs, typeRTLIL);
    inputs_.push_back(Gate::Signal::always(rightLeaf_));
    return net.addGate(func, inputs_);
  }
  if (!flag1 && flag2) {
    GateId leftLeaf_ = buildNet(leftLeaf, net, typeFunc, cell, inputs, typeRTLIL);
    inputs_.push_back(Gate::Signal::always(leftLeaf_));
    GateId rightLeaf_ = inputs.find(rightLeaf)->second;
    inputs_.push_back(Gate::Signal::always(rightLeaf_));
    return net.addGate(func, inputs_);
  }
  if (!flag1 && !flag2) {
    if (leftLeaf != rightLeaf) {
      GateId leftLeaf_ = buildNet(leftLeaf, net, typeFunc, cell, inputs, typeRTLIL);
      inputs_.push_back(Gate::Signal::always(leftLeaf_));
      GateId rightLeaf_ = buildNet(rightLeaf, net, typeFunc, cell, inputs, typeRTLIL);
      inputs_.push_back(Gate::Signal::always(rightLeaf_));
    } else {
      size_t singleLeaf = leftLeaf;
      GateId singleLeaf_ = buildNet(singleLeaf, net, typeFunc, cell, inputs, typeRTLIL);
      inputs_.push_back(Gate::Signal::always(singleLeaf_));
    }
    return net.addGate(func, inputs_);
  }
}

/// The function creates a tree for Liberty module.
static void printCells(
    const YLib::dict<RTlil::IdString, RTlil::Cell*> &cells,
    GModel::GNet &net,
    std::map<size_t, GateSymbol> &typeFunc,
    std::map<size_t, std::pair<size_t, size_t>> &cell,
    std::map<size_t, std::string> &typeRTLIL) {
  for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
    bool notOp = 0;
    GateSymbol f;
    size_t firstWire, secondWire, thirdWire;
    size_t typeFunction = it1->second->type.index_;
    auto it2 = it1->second->connections_.begin();
    auto endConnections = it1->second->connections_.end();
    size_t index = it2->second.as_wire()->name.index_;
    //the filling just cell
    if (it2 != endConnections) {
      firstWire = index;
      f = functionGateSymbol(typeFunction, typeRTLIL, firstWire);
      typeFunc.emplace(firstWire, f);
      if (f == GateSymbol::NOT) {
        notOp = 1;
      }
      ++it2;
    }
    if (it2 != endConnections) {
      index = it2->second.as_wire()->name.index_;
      if (!notOp) {
        secondWire = index;
        notOp = 0;
      } else {
        secondWire = index;
        thirdWire = secondWire;
        cell.emplace(firstWire, std::make_pair(secondWire, thirdWire));
      }
      ++it2;
    }
    if (it2 != endConnections) {
      thirdWire = it2->second.as_wire()->name.index_;
      cell.emplace(firstWire, std::make_pair(secondWire, thirdWire));
      ++it2;
    }
    // The filling specific cell using thirdWire such as the root of new cell.
    if (it2 != endConnections) {
      firstWire = it2->second.as_wire()->name.index_;
      ++it2;
    }
    if (it2 != it1->second->connections_.end()) {
      secondWire = it2->second.as_wire()->name.index_;
      cell.emplace(thirdWire, std::make_pair(firstWire, secondWire));
      f = functionGateSymbol(typeFunction, typeRTLIL, thirdWire);
      typeFunc.emplace(thirdWire, f);
    }
  }
}

/// The function finds output ID and determine root of tree for the output.
static void printConnections(
    const std::vector<std::pair<RTlil::SigSpec, RTlil::SigSpec>> &connections,
    GModel::GNet &net,
    std::map<size_t, GateSymbol> &typeFunc,
    std::map<size_t, std::pair<size_t, size_t>> &cell,
    std::map<size_t, GateId> &inputs,
    std::map<size_t, GateId> &outputs,
    std::map<size_t, std::string> &typeRTLIL) {
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    size_t root = it1->second.as_wire()->name.index_;
    // The case, when output is not equal input.
    if (inputs.find(root) == inputs.end()) {
      GateId output = buildNet(root, net, typeFunc, cell, inputs, typeRTLIL);
      outputs.find(root)->second = net.addOut(output);
    } else {
      GateId oneLeaf = inputs.find(root)->second;
      GateId output = net.addGate(GateSymbol::NOP, Gate::Signal::always(oneLeaf));
      outputs.find(root)->second = net.addOut(output);
    }
  }
}

void translateModuleToGNet(
    const std::pair<RTlil::IdString, RTlil::Module*> &m,
    GModel::GNet &net) {
  std::map<size_t, GateId> outputs;
  std::map<size_t, GateId> inputs;
  std::map<size_t, std::pair<size_t, size_t>> cell;
  std::map<size_t, GateSymbol> typeFunc;
  std::map<size_t, std::string> typeRTLIL;
  printWires(m.second->wires_, net, inputs, outputs);
  printCells(m.second->cells_, net, typeFunc, cell, typeRTLIL);
  printConnections(
      m.second->connections_,
      net,
      typeFunc,
      cell,
      inputs,
      outputs,
      typeRTLIL);
}

void translateDesignToGNet(
    const RTlil::Design &des,
    std::vector<GModel::GNet> &vec) {
  for (auto &m : des.modules_) {
    GModel::GNet net(0);
    translateModuleToGNet(m, net);
    vec.push_back(net);
  }
}
