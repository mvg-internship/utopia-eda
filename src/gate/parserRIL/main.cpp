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

void printWires(const YLib::dict<RTlil::IdString, RTlil::Wire *> &ywires,
                std::ofstream &fout,
                std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires) {

  for (auto it1 = ywires.begin(); it1 != ywires.end(); ++it1) {
    auto portOutput = it1->second->port_output;
    auto portInput = it1->second->port_input;
    unsigned index = it1->first.index_;
    std::string wireName = it1->first.str();
    std::string width = "u:";

    width.append(std::to_string(it1->second->width));
    if (it1->second->width > 1) {
      fout << "output " << width << " " << wireName << ";\n";
      outputs.emplace(index, wireName);
    }
    if (portInput == 1) {
      wireName.erase(0, 1);
      fout << "input " << width << " " << wireName << ";\n";
      inputs.emplace(index, wireName);
    }
    if (portOutput == 1) {
      wireName.erase(0, 1);
      fout << "output " << width << " " << wireName << ";\n";
      outputs.emplace(index, wireName);
    }
    if (portOutput == 0 && portInput == 0) {
      wires.emplace(index, wireName);
      fout << "wire " << width << " " << wireName << ";\n";
    }
  }

}

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

std::string buildRIL(int root, std::map<int, std::pair<int, int>> &cell,
                     std::map<int, std::string> &inputs,
                     std::map<int, std::string> &typeFunc) {

  auto rootCell = cell.at(root);
  bool flag1  = inputs.find(rootCell.first) != inputs.end();
  bool flag2  = inputs.find(rootCell.second) != inputs.end();
  auto cellPairFirst = cell.find(root)->first;
  auto cellPairSecond = cell.find(root)->second;
  auto leftLeaf = cellPairSecond.first;
  auto rightLeaf = cellPairSecond.second;
  std::string strCells = typeFunc.find(cellPairFirst)->second;
  if (flag1 && flag2) {
    if (leftLeaf != rightLeaf) {
      return inputs.find(leftLeaf)->second + strCells + inputs.find(rightLeaf)->second;
    } else {
      return strCells + inputs.find(rightLeaf)->second;
    }
  } else if (!flag1 && !flag2) {
    if (leftLeaf != rightLeaf) {
      return "(" +
             buildRIL(leftLeaf, cell, inputs, typeFunc) +
             strCells +
             buildRIL(rightLeaf, cell, inputs, typeFunc) +
             ")";
    } else {
      return strCells + "(" +
             buildRIL(leftLeaf, cell, inputs, typeFunc) +
             ")";
    }
  } else if (!flag1 && flag2) {
    return "(" +
           buildRIL(leftLeaf, cell, inputs, typeFunc) +
           strCells +
           inputs.find(rightLeaf)->second + ")";
  } else {
    return "(" + inputs.find(leftLeaf)->second +
           strCells +
           buildRIL(rightLeaf, cell, inputs, typeFunc) +
           ")";
  }

}

void printCells(const YLib::dict<RTlil::IdString, RTlil::Cell *> &cells,
                std::ofstream &fout,
                std::map<int, std::pair<int, int>> &cell,
                std::map<int, std::string> &typeFunc) {

  for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
    size_t i = 0;
    bool flag = 0;
    std::string symbol;
    int a, b, c;
    for (auto it3 = it1->second->connections_.begin();
         it3 != it1->second->connections_.end(); ++it3) {
      i++;
      auto connAsWire = it3->second.as_wire()->name.index_;
      if (i == 1) {
        a = connAsWire;
        symbol = logicFunction(it1->second->type.index_);
        typeFunc.emplace(a, symbol);
        if (symbol == "~") {
          flag = 1;
        }
      }
      if (i == 2) {
        if (!flag) {
          b = connAsWire;
          flag = 0;
        } else {
          b = connAsWire;
          c = b;
          cell.emplace(a, std::make_pair(b, c));
        }
      }
      if (i == 3) {
        c = connAsWire;
        cell.emplace(a, std::make_pair(b, c));
      }
      if (i == 4) {
        a = connAsWire;
      }
      if (i == 5) {
        b = connAsWire;
        cell.emplace(c, std::make_pair(a, b));
        typeFunc.emplace(c, symbol);
      }
    }
  }

}

void printConnections(
    const std::vector<std::pair<RTlil::SigSpec, RTlil::SigSpec>> &connections,
    std::ofstream &fout,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &inputs, std::map<int, std::string> &outputs,
    std::map<int, std::string> &typeFunc) {

  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    auto connAsWireInput = it1->second.as_wire()->name.index_;
    auto connAsWireOutput = it1->first.as_wire()->name.index_;
    if (inputs.find(connAsWireInput) == inputs.end()) {
      fout << "@(*) {\n";
      fout << "   " << outputs.find(connAsWireOutput)->second
           << " = " << buildRIL(connAsWireInput, cell, inputs, typeFunc) << ";\n";
      fout << "}\n";
    } else {
      fout << "@(*) {\n";
      fout << "   " << outputs.find(connAsWireOutput)->second
           << " = " << inputs.find(connAsWireInput)->second << ";\n";
      fout << "}\n";
    }
  }

}

void printActions(const std::vector<RTlil::SigSig> &actions,
                  std::ofstream &fout,
                  std::map<int, std::string> &inputs,
                  std::map<int, std::string> &outputs,
                  std::map<int, std::string> &wires) {

  int actNum = 0;
  for (auto &it : actions) {
    actNum++;

    bool firstIsWire = it.first.is_wire();
    bool secondIsWire = it.second.is_wire();
    bool firstIsChunk = it.first.is_chunk();
    bool secondIsChunk = it.second.is_chunk();

    if (firstIsChunk && secondIsChunk) {
      auto firstAsChunk = it.first.as_chunk().wire->name.index_;
      auto secondAsChunk = it.second.as_chunk().wire->name.index_;

      fout << "  ";
      if (wires.find(firstAsChunk) != wires.end()) {
        fout << wires.find(firstAsChunk)->second
             << " = ";
      }
      if (inputs.find(firstAsChunk) != inputs.end()) {
        fout << inputs.find(firstAsChunk)->second
             << " = ";
      }
      if (outputs.find(firstAsChunk) !=
          outputs.end()) {
        fout << outputs.find(firstAsChunk)->second
             << " = ";
      }
      if (wires.find(secondAsChunk) != wires.end()) {
        fout << wires.find(secondAsChunk)->second
             << "\n";
      }
      if (inputs.find(secondAsChunk) != inputs.end()) {
        fout << inputs.find(secondAsChunk)->second
             << "\n";
      }
      if (outputs.find(secondAsChunk) !=
          outputs.end()) {
        fout << outputs.find(secondAsChunk)->second
             << "\n";
      }
    }
  }

}

void printSyncs(const std::vector<RTlil::SyncRule *> &syncs,
                std::ofstream &fout,
                std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires, std::string &state,
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

    temporary = (*it1)->signal.as_wire()->name.index_;

    if ((*it1)->actions.size() != 0) {
      fout << "@(*) {\n";
      printActions((*it1)->actions, fout, inputs, outputs, wires);
      fout << "}\n";
    }
  }

}

void printCaseRule(std::vector<RTlil::CaseRule *> &cases,
                   std::ofstream &fout,
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

void printProcesses(
    const YLib::dict<RTlil::IdString, RTlil::Process *>
        &processes,
    std::ofstream &fout, std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs, std::map<int, std::string> &wires) {

  for (auto it1 = processes.begin(); it1 != processes.end(); ++it1) {
    std::string state;
    int temporary;
    printSyncs(it1->second->syncs, fout, inputs, outputs, wires, state, temporary);
    fout << "@(" << state << "(" << inputs.find(temporary)->second << ")) {\n";
    auto rootCase = it1->second->root_case;
    printActions(rootCase.actions, fout, inputs, outputs, wires);
    for (auto it2 : rootCase.switches) {
      printCaseRule(it2->cases, fout, inputs, outputs, wires);
    }
    fout << "}\n";
  }

}

void printParams(
    const std::pair<RTlil::IdString, RTlil::Module *> &m,
    std::ofstream &fout) {

  std::map<int, std::string> inputs;
  std::map<int, std::string> outputs;
  std::map<int, std::string> typeFunc;
  std::map<int, std::string> wires;
  std::map<int, std::pair<int, int>> cell;
  printWires(m.second->wires_, fout, inputs, outputs, wires);
  printCells(m.second->cells_, fout, cell, typeFunc);
  printConnections(m.second->connections_, fout, cell, inputs, outputs,
                   typeFunc);
  printProcesses(m.second->processes, fout, inputs, outputs, wires);

}
void printParsed(const RTlil::Design &des,
                 std::ofstream &fout) {

  for (auto &m : des.modules_) {
    printParams(m, fout);
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
    std::ofstream fout(filename);
    printParsed(design, fout);
    std::cout << "Parsed to: " << filename << "\n";
    fout.close();
  }
  Yosys::yosys_shutdown();

}
