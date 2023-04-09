//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright <2023> ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <kernel/yosys.h>

namespace RTlil = Yosys::RTLIL;
namespace YLib = Yosys::hashlib;

/// The function printins headers of ril file
void printHeaders(
    const YLib::dict<RTlil::IdString, RTlil::Wire *> &ywires,
    std::ostream &fout,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &wires) {
  for (auto it1 = ywires.begin(); it1 != ywires.end(); ++it1) {
    auto portOutput = it1->second->port_output;
    auto portInput = it1->second->port_input;
    auto portWire = it1->second->upto;
    unsigned index = it1->first.index_;
    std::string wireName = it1->first.str();
    int countDlr = std::count(wireName.begin(), wireName.end(), '$');
    std::string width = "u:";
    width.append(std::to_string(it1->second->width));
    if (portInput) {
      wireName.erase(0, 1);
      fout << "input " << width << " " << wireName << ";\n";
      inputs.emplace(index, wireName);
    }
    if (portOutput) {
      wireName.erase(0, 1);
      fout << "output " << width << " " << wireName << ";\n";
      outputs.emplace(index, wireName);
    }
    if (!portOutput && !portInput) {
      fout << "??? " << width << " " << wireName << ";\n";
      //wires.emplace(index, wireName);
      if (countDlr == 1) {
        //fout << "reg " << width << " " << wireName << ";\n";
        wireName.erase(4);
        wireName.erase(0, 3);
        //fout << "reg " << width << " " << wireName << ";\n";
        wires.emplace(index, wireName);
      } else if (countDlr == 0) {
        //fout << "wire " << width << " " << wireName << ";\n";
        wireName.erase(0, 1);
        //fout << "wire " << width << " " << wireName << ";\n";
        wires.emplace(index, wireName);
      }
    }
  }
}

/// The function determines operation
std::string logicFunction(size_t type) {
  std::string func;
  if (type == ID($add).index_) {
    func = "+";
  }
  if (type == ID($sub).index_) {
    func = "-";
  }
  if (type == ID($and).index_) {
    func = "&";
  }
  if (type == ID($or).index_) {
    func = "|";
  }
  if (type == ID($xor).index_) {
    func = "^";
  }
  if (type == ID($not).index_) {
    func = "~";
  }
  return func;
}

/// The function builds operation with variables
void buildRIL(
    std::ostream &fout,
    int root,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &typeFunc);

void printGate(
    std::ostream &fout,
    int node,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &typeFunc) {
  auto it = inputs.find(node);
  if (it != inputs.end()) {
    fout << it->second;
  } else {
    buildRIL(fout, node, cell, inputs, typeFunc);
  }
}

void buildRIL(
    std::ostream &fout,
    int root,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &typeFunc) {
  auto rootCell = cell.at(root);
  std::string opName = typeFunc.at(root);
  if (rootCell.first == rootCell.second) {
    fout << opName << "(";
    printGate(fout, rootCell.first, cell, inputs, typeFunc);
    fout << ")";
  } else {
    fout << "(";
    printGate(fout, rootCell.second, cell, inputs, typeFunc);
    fout << opName ;
    printGate(fout, rootCell.first, cell, inputs, typeFunc);
    fout << ")";
  }
}

/// The function fills cell map to be used in buildRil
void printCells(
    const YLib::dict<RTlil::IdString, RTlil::Cell *> &cells,
    std::ostream &fout,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &typeFunc) {
  for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
      bool flag = false;
      std::string symbol;
      int addrWire, dataWire, outWire;
      const auto &conns = it1->second->connections_;
      auto it = conns.begin();
      auto connAsWire = it->second.as_wire()->name.index_;
      if (it == conns.end()) {
        continue;
      }
      connAsWire = it->second.as_wire()->name.index_;
      addrWire = connAsWire;
      symbol = logicFunction(it1->second->type.index_);
      typeFunc.emplace(addrWire, symbol);
      if (symbol == "~") {
        flag = true;
      }
      if (++it == conns.end()) {
        continue;
      }
      connAsWire = it->second.as_wire()->name.index_;
      dataWire = connAsWire;
      if (flag) {
        outWire = dataWire;
        cell.emplace(addrWire, std::make_pair(dataWire, outWire));
      }
      if (++it == conns.end()) {
        continue;
      }
      connAsWire = it->second.as_wire()->name.index_;
      outWire = connAsWire;
      cell.emplace(addrWire, std::make_pair(dataWire, outWire));
      if (++it == conns.end()) {
        continue;
      }
      connAsWire = it->second.as_wire()->name.index_;
      addrWire = connAsWire;
      if (++it == conns.end()) {
        continue;
      }
      connAsWire = it->second.as_wire()->name.index_;
      dataWire = connAsWire;
      cell.emplace(outWire, std::make_pair(addrWire, dataWire));
      typeFunc.emplace(outWire, symbol);
  }
}

/// The function prints connections between variables
void printConnections(
    const std::vector<std::pair<RTlil::SigSpec, RTlil::SigSpec>> &connections,
    std::ostream &fout,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &typeFunc) {
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    auto connAsWireInput = it1->second.as_wire()->name.index_;
    auto connAsWireOutput = it1->first.as_wire()->name.index_;
    if (inputs.find(connAsWireInput) == inputs.end()) {
      fout << "@(*) {\n";
      fout << "  " << outputs[connAsWireOutput] << " = ";
      buildRIL(fout, connAsWireInput, cell, inputs, typeFunc);
      fout << ";\n}\n";
    } else {
      fout << "@(*) {\n";
      fout << "  " << outputs[connAsWireOutput] << " = "
           << inputs[connAsWireInput] << ";\n}\n";
    }
  }
}

/// The helper function for printActions
void printExistingAct(
    int key,
    const std::map<int, std::string> &type,
    std::string suffix,
    std::ostream &fout) {
  auto it = type.find(key);
  if (it != type.end()) {
    fout << it->second << suffix;
  }
}

/// The function prints actions between variables
void printActions(
    const std::vector<RTlil::SigSig> &actions,
    std::ostream &fout,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &wires) {
  for (auto &it : actions) {
    bool firstIsChunk = it.first.is_chunk();
    bool secondIsChunk = it.second.is_chunk();
    if (firstIsChunk && secondIsChunk) {
      auto firstAsChunk = it.first.as_chunk().wire->name.index_;
      auto secondAsChunk = it.second.as_chunk().wire->name.index_;
      fout << "  ";
      printExistingAct(firstAsChunk, wires, " = ", fout);
      printExistingAct(firstAsChunk, inputs, " = ", fout);
      printExistingAct(firstAsChunk, outputs, " = ", fout);
      printExistingAct(secondAsChunk, wires, ";\n", fout);
      printExistingAct(secondAsChunk, inputs, ";\n", fout);
      printExistingAct(secondAsChunk, outputs, ";\n", fout);
    }
  }
}

/// The function determines states
void detState(
    const std::vector<RTlil::SyncRule *> &syncs,
    std::ostream &fout,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &wires,
    std::string &state,
    int &temporary) {
  for (auto it1 = syncs.begin(); it1 != syncs.end(); ++it1) {
    std::string listSensitivity[]{
        "ST0", // level sensitive: 0
        "ST1", // level sensitive: 1
        "STp", // edge sensitive: posedge
        "STn", // edge sensitive: negedge
        "STe", // edge sensitive: both edges
        "STa", // always active
        "STi"  // init
    };
    std::string currType = listSensitivity[(*it1)->type];
    if (currType == "STn") {
      state = "negedge";
    }
    if (currType == "STp") {
      state = "posedge";
    }
    if (currType == "ST0") {
      state = "level0";
    }
    if (currType == "ST1") {
      state = "level1";
    }
    if (currType == "STa") {
      state = "always";
    }
    temporary = (*it1)->signal.as_wire()->name.index_;
  }
}

/// The function calling actions in some cases
void printCaseRule(
    std::vector<RTlil::CaseRule *> &cases,
    std::ostream &fout,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &wires) {
  for (auto i : cases) {
    printActions(i->actions, fout, inputs, outputs, wires);
    for (auto j : i->switches) {
      printCaseRule(j->cases, fout, inputs, outputs, wires);
    }
  }
}

/// The function printing states
void printProcesses(
    const YLib::dict<RTlil::IdString, RTlil::Process *> &processes,
    std::ostream &fout,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &wires) {
  for (auto it1 = processes.begin(); it1 != processes.end(); ++it1) {
    std::string state;
    int temporary;
    detState(it1->second->syncs, fout, inputs, outputs,
             wires, state, temporary);
    fout << "@(" << state << "(" << inputs[temporary] << ")) {\n";
    auto rootCase = it1->second->root_case;
    printActions(rootCase.actions, fout, inputs, outputs, wires);
    for (auto it2 : rootCase.switches) {
      printCaseRule(it2->cases, fout, inputs, outputs, wires);
    }
    fout << "}\n";
  }
}

/// The function creates variables to be filled and calls other functions
void printParams(const RTlil::Design &des, std::ostream &fout) {
  for (auto &m : des.modules_) {
    std::map<int, std::string> inputs;
    std::map<int, std::string> outputs;
    std::map<int, std::string> typeFunc;
    std::map<int, std::string> wires;
    std::map<int, std::pair<int, int>> cell;
    printHeaders(m.second->wires_, fout, inputs, outputs, wires);
    printCells(m.second->cells_, fout, cell, typeFunc);
    printConnections(m.second->connections_, fout, cell, inputs, outputs,
                     typeFunc);
    printProcesses(m.second->processes, fout, inputs, outputs, wires);
  }
}

int main(int argc, char *argv[]) {
  Yosys::yosys_setup();
  for (size_t o = 1; o < argc; ++o) {
    std::string filename = argv[o];
    std::string extension = ".ril";
    RTlil::Design design;
    Yosys::run_frontend(filename, "verilog", &design, nullptr);
    size_t pos = filename.rfind(".");
    if (pos != std::string::npos) {
      filename.replace(pos, extension.length(), extension);
    }
    std::filebuf fb;
    fb.open(filename, std::ios::out);
    std::ostream fout(&fb);
    printParams(design, fout);
    std::cout << "Parsed to: " << filename << "\n";
    fb.close();
  }
  Yosys::yosys_shutdown();
}
