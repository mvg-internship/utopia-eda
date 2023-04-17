//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright <2023> ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "../src/rtl/model/net.h"
#include "../src/rtl/model/pnode.h"
#include "../src/rtl/model/vnode.h"
#include "../src/rtl/parser/ril/parser.h"
#include <kernel/yosys.h>

namespace RTlil = Yosys::RTLIL;
namespace YLib = Yosys::hashlib;

std::string TMP_WIRES = "_1";
void printModules(const RTlil::IdString module, std::ostream &out,
                  std::ofstream &fout) {

  out << module.str() << " - --module of index: " << module.index_ << "\n";
}

void printWires(const YLib::dict<RTlil::IdString, RTlil::Wire *> &ywires,
                std::ostream &out, std::ofstream &fout,
                std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires) {

  out << "Wires:"
      << "\n";
  for (auto it1 = ywires.begin(); it1 != ywires.end(); ++it1) {
    auto portOutput = it1->second->port_output;
    auto portInput = it1->second->port_input;
    unsigned index = it1->first.index_;
    std::string wireName = it1->first.str();
    std::string width = "u:";

    out << "  " << wireName << " - wire of index: " << index << "\n";

    width.append(std::to_string(it1->second->width));
    if (it1->second->width > 1) {
      out << "    type: bus with width " << it1->second->width << "\n";
      fout << "output " << width << " " << wireName << ";\n";
      outputs.emplace(index, wireName);
    } else {
      out << "    type: wire"
          << "\n";
    }
    out << "    start_offset: " << it1->second->start_offset << "\n";
    out << "    port_id: " << it1->second->port_id << "\n";
    out << "    port_input: " << portInput << "\n";
    if (portInput == 1) {
      wireName.erase(0, 1);
      fout << "input " << width << " " << wireName << ";\n";
      inputs.emplace(index, wireName);
    }
    out << "    port_output: " << portOutput << "\n";
    if (portOutput == 1) {
      wireName.erase(0, 1);
      fout << "output " << width << " " << wireName << ";\n";
      outputs.emplace(index, wireName);
    }
    if (portOutput == 0 && portInput == 0) {
      wires.emplace(index, TMP_WIRES);
      TMP_WIRES += "1";
    }
    out << "    upto: " << it1->second->upto << "\n\n";
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
  bool flag1 = inputs.find(rootCell.first) != inputs.end();
  bool flag2 = inputs.find(rootCell.second) != inputs.end();
  auto cellPairFirst = cell.find(root)->first;
  auto cellPairSecond = cell.find(root)->second;
  auto leftLeaf = cellPairSecond.first;
  auto rightLeaf = cellPairSecond.second;
  std::string strCells = typeFunc.find(cellPairFirst)->second;
  if (flag1 && flag2) {
    if (leftLeaf != rightLeaf) {
      return inputs.find(leftLeaf)->second + strCells +
             inputs.find(rightLeaf)->second;
    } else {
      return strCells + inputs.find(rightLeaf)->second;
    }
  } else if (!flag1 && !flag2) {
    if (leftLeaf != rightLeaf) {
      return "(" + buildRIL(leftLeaf, cell, inputs, typeFunc) + strCells +
             buildRIL(rightLeaf, cell, inputs, typeFunc) + ")";
    } else {
      return strCells + "(" + buildRIL(leftLeaf, cell, inputs, typeFunc) + ")";
    }
  } else if (!flag1 && flag2) {
    return "(" + buildRIL(leftLeaf, cell, inputs, typeFunc) + strCells +
           inputs.find(rightLeaf)->second + ")";
  } else {
    return "(" + inputs.find(leftLeaf)->second + strCells +
           buildRIL(rightLeaf, cell, inputs, typeFunc) + ")";
  }
}

void printCells(const YLib::dict<RTlil::IdString, RTlil::Cell *> &cells,
                std::ostream &out, std::ofstream &fout,
                std::map<int, std::pair<int, int>> &cell,
                std::map<int, std::string> &typeFunc) {

  out << "Cells:"
      << "\n";
  for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
    out << "  " << it1->first.str() << " cell of index " << it1->first.index_
        << " type " << it1->second->type.index_ << "\n";

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
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    auto connAsWireInput = it1->second.as_wire()->name.index_;
    auto connAsWireOutput = it1->first.as_wire()->name.index_;
    out << "    Wire " << connAsWireOutput << " connects with "
        << connAsWireInput << "\n";
    if (inputs.find(connAsWireInput) == inputs.end()) {
      fout << "@(*) {\n";
      fout << "   " << outputs.find(connAsWireOutput)->second << " = "
           << buildRIL(connAsWireInput, cell, inputs, typeFunc) << ";\n";
      fout << "}\n";
    } else {
      fout << "@(*) {\n";
      fout << "   " << outputs.find(connAsWireOutput)->second << " = "
           << inputs.find(connAsWireInput)->second << ";\n";
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

bool FF = false;
std::string temp1 = "", temp2 = "";
std::vector<std::string> listSens;
std::string cond = "";
bool DFF = false;
std::string listFF = "";
void printSyncs(const std::vector<RTlil::SyncRule *> &syncs, std::ostream &out,
                std::ofstream &fout, std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires, std::string &state,
                int &temporary, std::map<int, std::pair<int, int>> &cell,
                std::map<int, std::string> &typeFunc) {
  out << "Syncs: \n";
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
    std::string state;
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
      state = "*";
      // flag=1;
    }

    std::string inp = Yosys::log_signal((*it1)->signal);
    inp.erase(0, 1);
    listSens.push_back("@(" + state + "(" + inp + ")");
    if (state != "*")
      listFF = "@(" + state + "(" + inp + ")) {";
    else
      listFF = "@(*) {";
    out << "   " << currType << "\n";
    out << "    " << Yosys::log_signal((*it1)->signal) << "\n";
    out << "    actions:\n" << FF;
    if (FF)
      for (auto it2 : (*it1)->actions) {
        // fout << "zzzzzz\n";
        // fout << listSensitivity << "{\n" << Yosys::log_signal((it2).first) <<
        // " = " << Yosys::log_signal((it2).second)<<";\n}\n";
      }

    for (auto it2 : (*it1)->actions) {
      out << "      " << Yosys::log_signal((it2).first) << "      "
          << Yosys::log_signal((it2).second) << "\n";
      std::string t1 = Yosys::log_signal((it2).first),
                  t2 = Yosys::log_signal((it2).second);
      out << ((temp1 != t1) || (temp2 != t2)) << "\n";
      if ((temp1 != t1) || (temp2 != t2)) {
        t1.erase(0, 1);
        t2.erase(0, 1);
        if (it2.second.is_wire()) {
          fout << "wire u:" << it2.second.as_wire()->width << " "
               << wires[it2.second.as_wire()->name.index_] << ";\n";
          fout << "@(*) {\n   " << t1 << " = "
               << wires[it2.second.as_wire()->name.index_] << ";\n}\n";
        }
      }
      temp1 = Yosys::log_signal((it2).first);
      temp2 = Yosys::log_signal((it2).second);
    }
  }
  // listSens.erase(listSens.size()-2,listSens.size()-1);
}

void printCompare(const std::vector<RTlil::SigSpec> &compare, std::ostream &out,
                  std::ofstream &fout, std::map<int, std::string> &inputs,
                  std::map<int, std::string> &outputs,
                  std::map<int, std::string> &wires) {
  out << "compareHere:\n" << compare.size();
  for (auto &it : compare) {
    out << "    " << Yosys::log_signal(it) << "\n";
  }
  out << "enDcompare:\n";
}
int i = 0;
bool Flag = false;
int width;
std::string func = "";
bool flagg=false;
void printActions(const std::vector<RTlil::SigSig> &actions, std::ostream &out,
                  std::ofstream &fout, std::map<int, std::string> &inputs,
                  std::map<int, std::string> &outputs,
                  std::map<int, std::string> &wires,
                  std::map<int, std::pair<int, int>> &cell,
                  std::map<int, std::string> &typeFunc) {
  out << "here\n";
  std::string tmp1 = "";
  for (auto &it : actions) {
      int z = 0;
      for (auto itt : listSens) {
    out << "    " << Yosys::log_signal(it.first) << "    "
        << Yosys::log_signal(it.second) << "\n";
    tmp1 = Yosys::log_signal(it.first);
    out << "AAAAAAAAAAAAAAAA" << (tmp1.find('{') != std::string::npos) << "\n";
    bool cf = tmp1.find('{') != std::string::npos;
    if (DFF) {
      std::string tmp1 = Yosys::log_signal(it.first);
      bool cf = tmp1.find('{') != std::string::npos;
      if (!cf) {
        if (wires.find(it.first.as_wire()->name.index_) != wires.end())
          fout << listFF << "\n    " << wires[it.first.as_wire()->name.index_]
               << " = ";
        else
          fout << listFF << "\n    " << outputs[it.first.as_wire()->name.index_]
               << " = ";
        if (cell.find(it.second.as_wire()->name.index_) != cell.end())
          fout << buildRIL(it.second.as_wire()->name.index_, cell, inputs,
                           typeFunc)
               << ";\n}\n";
        else if (inputs.find(it.second.as_wire()->name.index_) != inputs.end())
          fout << inputs[it.second.as_wire()->name.index_] << ";\n}\n";
        else if (wires.find(it.second.as_wire()->name.index_) != wires.end())
          fout << wires[it.second.as_wire()->name.index_] << ";\n}\n";
        else
          fout << Yosys::log_signal(it.second) << ";\n}\n";
      }
    }
    if (!cf && !DFF) {

        if (!it.first.has_marked_bits()) {
          if (FF) {

            if (it.first.is_wire() && it.second.is_wire()) {
              if (wires.find(it.first.as_wire()->name.index_) != wires.end()) {

                fout << itt << ") {\n"
                     << "    " << wires[it.first.as_wire()->name.index_]
                     << " = ";
              } else {
                fout << itt << ") {\n"
                     << "    " << outputs[it.first.as_wire()->name.index_]
                     << " = ";
              }
              if (it.second.is_wire()) {
                if (cell.find(it.second.as_wire()->name.index_) != cell.end()) {
                  fout << buildRIL(it.second.as_wire()->name.index_, cell,
                                   inputs, typeFunc)
                       << ";\n}\n";
                } else if (wires.find(it.second.as_wire()->name.index_) !=
                           wires.end()) {
                  fout << wires[it.second.as_wire()->name.index_] << ";\n}\n";
                } else if (inputs.find(it.second.as_wire()->name.index_) !=
                           inputs.end()) {
                  fout << inputs[it.second.as_wire()->name.index_] << ";\n}\n";
                }
              } else {
                fout << Yosys::log_signal(it.second) << ";\n}\n";
              }
            }
          }
          if (it.first.is_wire()) {
            if (Flag && (tmp1 != "")) {

              if (i == 0 || i == 1) {
                if (i==0)
                 cond.erase(0, 1);
                fout << itt << ") "
                     << "if (" << cond << ") {\n";

                if (i==1)
                func += "~" + cond + ")";

                if (!it.second.is_wire()) {
                  fout << "    " << wires[it.first.as_wire()->name.index_]
                       << " = " << Yosys::log_signal(it.second) << ";\n}\n";
                } else {
                  if (cell.find(it.second.as_wire()->name.index_) != cell.end())
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = "
                         << buildRIL(it.second.as_wire()->name.index_, cell,
                                     inputs, typeFunc)
                         << ";\n}\n";
                  else
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = " << inputs[it.second.as_wire()->name.index_]
                         << ";\n}\n";
                }
              }
              if (i == 2||i == 3) {
                // fout << "UUUUUUU" << cond << " " << func;

                  func += cond.erase(0, 1);
                  if (i==2)
                func.erase(func.size() - 1, func.size());
                z++;

                fout << "wire u:" << width << " " << TMP_WIRES << ";\n";
                fout << "@(*) {\n   " << TMP_WIRES << " = " << func << ";\n}\n";

                fout << itt << ") "
                     << "if (" << TMP_WIRES << ") {\n";

                if (!it.second.is_wire()) {
                  fout << "    " << wires[it.first.as_wire()->name.index_]
                       << " = " << Yosys::log_signal(it.second) << ";\n}\n";
                } else {
                  if (cell.find(it.second.as_wire()->name.index_) != cell.end())
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = "
                         << buildRIL(it.second.as_wire()->name.index_, cell,
                                     inputs, typeFunc)
                         << ";\n}\n";
                  else
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = " << inputs[it.second.as_wire()->name.index_]
                         << ";\n}\n";
                }
                TMP_WIRES += "1";
              }
              if (i == 4 || i==5) {
                func.erase(func.size() - 2, func.size() - 1);
                func += "&(~" + cond + ")";
                fout << "wire u:" << width << " " << TMP_WIRES << ";\n";
                fout << "@(*) {\n   " << TMP_WIRES << " = " << func << ";\n}\n";
                fout << itt << ") "
                     << "if (" << TMP_WIRES << ") {\n";
                if (!it.second.is_wire()) {
                  fout << "    " << wires[it.first.as_wire()->name.index_]
                       << " = " << Yosys::log_signal(it.second) << ";\n}\n";
                } else {
                  if (cell.find(it.second.as_wire()->name.index_) != cell.end())
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = "
                         << buildRIL(it.second.as_wire()->name.index_, cell,
                                     inputs, typeFunc)
                         << ";\n}\n";
                  else
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = " << inputs[it.second.as_wire()->name.index_]
                         << ";\n}\n";
                }
                TMP_WIRES += "1";
              }
              i++;
            }
          }
        }
      }
      else {
        out << "SKDLFFLSDFSDysys\n";
        DFF = true;
        continue;
      }
    }
  }
  for (auto &it : actions) {
    tmp1 = Yosys::log_signal(it.first);
    Flag = true;
  }
}
void printSwitches(std::vector<RTlil::SwitchRule *> &switches,
                   std::ostream &out, std::ofstream &fout,
                   std::map<int, std::string> &inputs,
                   std::map<int, std::string> &outputs,
                   std::map<int, std::string> &wires,
                   std::map<int, std::pair<int, int>> &cell,
                   std::map<int, std::string> &typeFunc) {
  out << switches.empty() << "\n";
  for (auto &it : switches) {
    out << "signal:\n";
    cond = Yosys::log_signal(it->signal);
    if (it->signal.is_wire())
      width = it->signal.as_wire()->width;
    out << "    sss" << Yosys::log_signal(it->signal) << "\n";
    for (auto it1 : it->cases) {
      out << "    ";

      printCompare(it1->compare, out, fout, inputs, outputs, wires);
      out << "    ";
      printActions(it1->actions, out, fout, inputs, outputs, wires, cell,
                   typeFunc);
      out << "    ";
      printSwitches(it1->switches, out, fout, inputs, outputs, wires, cell,
                    typeFunc);
    }
  }
}
void printProcesses(
    const YLib::dict<RTlil::IdString, RTlil::Process *> &processes,
    std::ostream &out, std::ofstream &fout, std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs, std::map<int, std::string> &wires,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &typeFunc) {

  out << "Processes count: " << processes.size() << "\n";
  size_t pp = 0;
  for (auto it1 = processes.begin(); it1 != processes.end(); ++it1) {
    pp++;
    out << "Process number " << pp << "\n";
    out << "  name " << it1->second->name.index_ << "\n";

    std::string state;
    int temporary;
    auto rootCase = it1->second->root_case;
    printSyncs(it1->second->syncs, out, fout, inputs, outputs, wires, state,
               temporary, cell, typeFunc);
    out << "  root_case: \ncompare: \n";
    printCompare(rootCase.compare, out, fout, inputs, outputs, wires);
    out << "\n";
    if (rootCase.switches.empty()) {
      FF = 1;
    }
    printActions(rootCase.actions, out, fout, inputs, outputs, wires, cell,
                 typeFunc);
    printSwitches(rootCase.switches, out, fout, inputs, outputs, wires, cell,
                  typeFunc);
  }
}

void printPorts(const std::vector<RTlil::IdString> &ports, std::ostream &out,
                std::ofstream &fout) {
  out << "Ports count: " << ports.size() << "\n";
  for (auto &it1 : ports) {
    out << "  port " << it1.index_ << "\n";
  }
}

void printParams(const std::pair<RTlil::IdString, RTlil::Module *> &m,
                 std::ostream &out, std::ofstream &fout) {

  std::map<int, std::string> inputs;
  std::map<int, std::string> outputs;
  std::map<int, std::string> typeFunc;
  std::map<int, std::string> wires;
  std::map<int, std::pair<int, int>> cell;
  printModules(m.first, out, fout);
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
  printProcesses(m.second->processes, out, fout, inputs, outputs, wires, cell,
                 typeFunc);
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
    //std::unique_ptr<eda::rtl::model::Net> tttest =
        //eda::rtl::parser::ril::parse(filename);
  }
  Yosys::yosys_shutdown();
}
