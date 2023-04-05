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

void printModules(const int ind, std::ostream &out, std::ofstream &fout) {

  for (const auto &it1 : RTlil::IdString::global_id_index_) {
    if (it1.second == ind) {
      out << it1.first << " - module of index: " << it1.second << "\n";
    }
  }

}

void printWires(const YLib::dict<RTlil::IdString, RTlil::Wire *> &ywires,
                std::ostream &out, std::ofstream &fout,
                std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires) {

  out << "Wires:" << "\n";
  for (auto it1 = ywires.begin(); it1 != ywires.end(); ++it1) {
    std::string temp;
    std::string width = "u:";
    unsigned index;
    auto portOutput = it1->second->port_output;
    auto portInput = it1->second->port_input;
    for (const auto &it2 : RTlil::IdString::global_id_index_) {
      if (it2.second == it1->first.index_) {
        out << "  " << it2.first << " - wire of index: " << it2.second << "\n";
        index = it2.second;
        temp = it2.first;
      }
    }
    width.append(std::to_string(it1->second->width));
    if (it1->second->width > 1) {
      out << "    type: bus with width " << it1->second->width << "\n";
      fout << "output " << width << " " << temp << ";\n";
      outputs.emplace(index, temp);
    } else {
      out << "    type: wire" << "\n";
    }
    out << "    start_offset: " << it1->second->start_offset << "\n";
    out << "    port_id: " << it1->second->port_id << "\n";
    out << "    port_input: " << portInput << "\n";
    if (portInput == 1) {
      temp.erase(0, 1);
      fout << "input " << width << " " << temp << ";\n";
      inputs.emplace(index, temp);
    }
    out << "    port_output: " << portOutput << "\n";
    if (portOutput == 1) {
      temp.erase(0, 1);
      fout << "output " << width << " " << temp << ";\n";
      outputs.emplace(index, temp);
    }
    if (portOutput == 0 && portInput == 0) {
      wires.emplace(index, temp);
      fout << "wire " << width << " " << temp << ";\n";
    }
    out << "    upto: " << it1->second->upto << "\n\n";
  }

}

std::string logicFunction(size_t type) {

  std::string func;
  if (type == ID($and).index_) {
    func = "&";
  }
  if (type == ID($not).index_) {
    func = "~";
  }
  if (type == ID($or).index_) {
    func = "|";
  }
  if (type == ID($xor).index_) {
    func = "^";
  }
  return func;

}

std::string buildRIL(int root, std::map<int, std::pair<int, int>> &cell,
                     std::map<int, std::string> &inputs,
                     std::map<int, std::string> &typeFunc) {

  bool flag1 = 0, flag2 = 0;
  auto cellPairFirst = cell.find(root)->first;
  auto cellPairSecond = cell.find(root)->second;
  if (inputs.find(cellPairSecond.first) != inputs.end()) {
    flag1 = 1;
  }
  if (inputs.find(cellPairSecond.second) != inputs.end()) {
    flag2 = 1;
  }
  if (flag1 == 1 && flag2 == 1) {
    if (cellPairSecond.first != cellPairSecond.second) {
      return inputs.find(cellPairSecond.first)->second +
             typeFunc.find(cellPairFirst)->second +
             inputs.find(cellPairSecond.second)->second;
    } else {
      return typeFunc.find(cellPairFirst)->second +
             inputs.find(cellPairSecond.second)->second;
    }
  }
  if (flag1 == 0 && flag2 == 0) {
    if (cellPairSecond.first != cellPairSecond.second) {
      return "(" +
             buildRIL(cellPairSecond.first, cell, inputs, typeFunc) +
             typeFunc.find(cellPairFirst)->second +
             buildRIL(cellPairSecond.second, cell, inputs, typeFunc) +
             ")";
    } else {
      return typeFunc.find(cellPairFirst)->second + "(" +
             buildRIL(cellPairSecond.first, cell, inputs, typeFunc) +
             ")";
    }
  }
  if (flag1 == 0 && flag2 == 1) {
    return "(" +
           buildRIL(cellPairSecond.first, cell, inputs, typeFunc) +
           typeFunc.find(cellPairFirst)->second +
           inputs.find(cellPairSecond.second)->second + ")";
  }
  if (flag1 == 1 && flag2 == 0) {
    return "(" + inputs.find(cellPairSecond.first)->second +
           typeFunc.find(cellPairFirst)->second +
           buildRIL(cellPairSecond.second, cell, inputs, typeFunc) +
           ")";
  }
  return "";

}

void printCells(const YLib::dict<RTlil::IdString, RTlil::Cell *> &cells,
                std::ostream &out, std::ofstream &fout,
                std::map<int, std::pair<int, int>> &cell,
                std::map<int, std::string> &typeFunc) {

  out << "Cells:" << "\n";
  for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
    for (const auto &it2 : RTlil::IdString::global_id_index_) {
      if (it2.second == it1->first.index_) {
        out << "  " << it2.first << " cell of index " << it2.second << " type "
            << it1->second->type.index_ << "\n";
      }
    }
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
      out << "    Connections: " << it3->first.index_ << "\n";
      out << "      name: " << connAsWire << "\n";
    }
  }

}

void printConnections(
    const std::vector<std::pair<RTlil::SigSpec, RTlil::SigSpec>> &connections,
    std::ostream &out, std::ofstream &fout,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &inputs, std::map<int, std::string> &outputs,
    std::map<int, std::string> &typeFunc) {

  out << "Connections count: " << connections.size() << "\n";
  int root;
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    auto connAsWireInput = it1->second.as_wire()->name.index_;
    auto connAsWireOutput = it1->first.as_wire()->name.index_;
    out << "    Wire " << connAsWireOutput << " connects with "
        << connAsWireInput << "\n";
    if (inputs.find(connAsWireInput) == inputs.end()) {
      root = connAsWireInput;
      fout << "@(*) {\n";
      fout << "   " << outputs.find(connAsWireOutput)->second
           << " = " << buildRIL(root, cell, inputs, typeFunc) << ";\n";
      fout << "}\n";
    } else {
      root = connAsWireInput;
      fout << "@(*) {\n";
      fout << "   " << outputs.find(connAsWireOutput)->second
           << " = " << inputs.find(root)->second << ";\n";
      fout << "}\n";
    }
  }

}

void printMemory(const YLib::dict<RTlil::IdString, RTlil::Memory *> &memories,
                 std::ostream &out, std::ofstream &fout) {

  out << "Memory count: " << memories.size() << "\n";
  for (auto it1 = memories.begin(); it1 != memories.end(); ++it1) {
    out << "  memory " << it1->second << "\n";
    out << "  name: " << it1->second->name.index_ << "\n";
    out << "  width: " << it1->second->width << "\n";
    out << "  start_offset: " << it1->second->start_offset << "\n";
    out << "  size: " << it1->second->size << "\n";
    out << "\n";
  }

}

void printActions(const std::vector<RTlil::SigSig> &actions,
                  std::ostream &out, std::ofstream &fout,
                  std::map<int, std::string> &inputs,
                  std::map<int, std::string> &outputs,
                  std::map<int, std::string> &wires) {

  int z = 0;
  out << "actions:\n";
  for (auto &it : actions) {
    z++;
    out << z << "\n";

    bool firstIsWire = it.first.is_wire();
    bool secondIsWire = it.second.is_wire();
    bool firstIsChunk = it.first.is_chunk();
    bool secondIsChunk = it.second.is_chunk();

    out << "    ch" << firstIsWire << " " << secondIsWire
        << "\n";
    if (firstIsWire && !secondIsWire) {
      out << "    nech" << it.first.as_wire()->name.index_ << "\n";
    }
    if (!firstIsWire && secondIsWire) {
      out << "    nech" << it.second.as_wire()->name.index_ << "\n";
    }
    if (firstIsWire && secondIsWire) {
      out << "    nech" << it.second.as_wire()->name.index_ << " "
          << it.second.as_wire()->name.index_ << "\n";
    }

    out << "    ch" << firstIsChunk << " " << secondIsChunk
        << "\n";

    if (firstIsChunk && !secondIsChunk) {
      out << "    nech" << (it.first.as_chunk().wire->name.index_) << "\n";
    }
    if (!firstIsChunk && secondIsChunk) {
      out << "    nech" << (it.second.as_chunk().wire->name.index_) << "\n";
    }
    if (firstIsChunk && secondIsChunk) {

      auto firstAsChunk = it.first.as_chunk().wire->name.index_;
      auto secondAsChunk = it.second.as_chunk().wire->name.index_;

      out << "    nech" << (it.first.as_chunk().wire)->name.index_ << " "
          << (it.second.as_chunk().wire)->name.index_ << "\n";
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
                std::ostream &out, std::ofstream &fout,
                std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires, std::string &state,
                int &temporary) {

  out << "  syncs\n";
  for (auto it1 = syncs.begin(); it1 != syncs.end(); ++it1) {
    out << "    type " << sizeof((*it1)->type) << "\n";
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

    out << currType << "\n";
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
    out << "  signal\n";
    out << "    size " << (*it1)->signal.size() << "\n";
    out << "    as_wire index " << temporary << "\n";
    out << "    as_chunk index " << (*it1)->signal.as_chunk().data.size()
        << "\n";
    out << "  actions\n";
    if ((*it1)->actions.size() != 0) {
      fout << "@(*) {\n";
      printActions((*it1)->actions, out, fout, inputs, outputs, wires);
      fout << "}\n";
    }
  }

}

void printSigSpec(const std::vector<RTlil::SigSpec> &compare,
                  std::ostream &out, std::map<int, std::string> &inputs,
                  std::map<int, std::string> &outputs,
                  std::map<int, std::string> &wires) {

  out << "compare:\n";
  for (auto &it : compare) {
    out << "    ch" << it.is_wire() << "\n";
    if (it.is_wire()) {
      out << "    nech" << it.as_wire()->name.index_ << "\n";
    }
    out << "    ch" << it.is_chunk() << "\n";
    if (it.is_chunk()) {
      out << "    nech" << it.as_chunk().wire->name.index_ << "\n";
    }
  }

}

void printCaseRule(std::vector<RTlil::CaseRule *> &cases,
                   std::ostream &out, std::ofstream &fout,
                   std::map<int, std::string> &inputs,
                   std::map<int, std::string> &outputs,
                   std::map<int, std::string> &wires);

void printSwitches(std::vector<RTlil::SwitchRule *> &switches,
              std::ostream &out, std::ofstream &fout,
              std::map<int, std::string> &inputs,
              std::map<int, std::string> &outputs,
              std::map<int, std::string> &wires) {

  out << "\n\n    switches:\n\n";
  for (auto i : switches) {
    out << "sw:";
    auto it = i->signal;
    out << "    ch" << it.is_wire() << "\n";
    if (it.is_wire()) {
      out << "    nech" << it.as_wire()->name.index_ << "\n";
    }
    out << "    ch" << it.is_chunk() << "\n";
    if (it.is_chunk()) {
      out << "    nech" << it.as_chunk().wire->name.index_ << "\n";
    }
    printCaseRule(i->cases, out, fout, inputs, outputs, wires);
  }

}

void printCaseRule(std::vector<RTlil::CaseRule *> &cases,
                   std::ostream &out, std::ofstream &fout,
                   std::map<int, std::string> &inputs,
                   std::map<int, std::string> &outputs,
                   std::map<int, std::string> &wires) {

  out << "CaseRule:\n";
  for (auto i : cases) {
    printSigSpec(i->compare, out, inputs, outputs, wires);
    printActions(i->actions, out, fout, inputs, outputs, wires);
    printSwitches(i->switches, out, fout, inputs, outputs, wires);
  }

}

void printProcesses(
    const YLib::dict<RTlil::IdString, RTlil::Process *>
        &processes,
    std::ostream &out, std::ofstream &fout, std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs, std::map<int, std::string> &wires) {

  out << "Processes count: " << processes.size() << "\n";
  for (auto it1 = processes.begin(); it1 != processes.end(); ++it1) {
    out << "  name " << it1->second->name.index_ << "\n";
    out << "  root_case: \n";
    std::string state;
    int temporary;
    printSyncs(it1->second->syncs, out, fout, inputs, outputs, wires, state, temporary);
    fout << "@(" << state << "(" << inputs.find(temporary)->second << ")) {\n";
    auto rootCase = it1->second->root_case;
    printSigSpec(rootCase.compare, out, inputs, outputs, wires);
    printActions(rootCase.actions, out, fout, inputs, outputs, wires);
    printSwitches(rootCase.switches, out, fout, inputs, outputs, wires);
    fout << "}\n";
    out << "\n";
  }

}

void printPorts(const std::vector<RTlil::IdString> &ports,
                std::ostream &out, std::ofstream &fout) {

  out << "Ports count: " << ports.size() << "\n";
  for (auto &it1 : ports) {
    out << "  port " << it1.index_ << "\n";
  }

}

void printParams(
    const std::pair<RTlil::IdString, RTlil::Module *> &m,
    std::ostream &out, std::ofstream &fout) {

  std::map<int, std::string> inputs;
  std::map<int, std::string> outputs;
  std::map<int, std::string> typeFunc;
  std::map<int, std::string> wires;
  std::map<int, std::pair<int, int>> cell;
  printModules(m.first.index_, out, fout);
  out << "\n";
  printWires(m.second->wires_, out, fout, inputs, outputs, wires);
  out << "\n";
  printCells(m.second->cells_, out, fout, cell, typeFunc);
  out << "\n";
  printConnections(m.second->connections_, out, fout, cell, inputs, outputs,
                   typeFunc);
  out << "\n";
  printMemory(m.second->memories, out, fout);
  out << "\n";
  printProcesses(m.second->processes, out, fout, inputs, outputs, wires);
  out << "\n";
  printPorts(m.second->ports, out, fout);
  out << "\n";
  out << "Refcount wires count: " << m.second->refcount_wires_ << "\n";
  out << "Refcount cells count: " << m.second->refcount_cells_ << "\n";
  out << "Monitors count: " << m.second->monitors.size() << "\n";
  out << "Avail_parameters count: " << m.second->avail_parameters.size()
      << "\n";
  out << "\n";

}
void printParsed(const RTlil::Design &des, std::ostream &out,
                 std::ofstream &fout) {

  for (auto &m : des.modules_) {
    printParams(m, out, fout);
  }

}
int main(int argc, char *argv[]) {

  std::ostream &out = std::cout;
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
    printParsed(design, out, fout);
    fout.close();
  }
  Yosys::yosys_shutdown();

}
