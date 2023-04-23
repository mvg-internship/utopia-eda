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
#include <set>

namespace RTlil = Yosys::RTLIL;
namespace YLib = Yosys::hashlib;

void printWires(const YLib::dict<RTlil::IdString,
                RTlil::Wire *> &ywires,
                std::map<std::string, int> &widt,
                std::ofstream &fout,
                std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires,
                std::string &TMP_WIRES) {
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
      wires.emplace(index, TMP_WIRES);
      widt.emplace(TMP_WIRES, it1->second->width);
      // fout << "wire " << width << " " << TMP_WIRES << ";\n";
      TMP_WIRES += "1";
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

std::string buildRIL(int root,
                     std::map<int, std::pair<int, int>> &cell,
                     std::map<int, std::string> &inputs,
                     std::map<int, std::string> &typeFunc,
                     std::map<int, std::string> &wires) {
  auto rootCell = cell.at(root);
  bool flag1 = inputs.find(rootCell.first) != inputs.end();
  bool flag2 = inputs.find(rootCell.second) != inputs.end();
  auto cellPairSecond = cell[root];
  auto leftLeaf = cellPairSecond.first;
  auto rightLeaf = cellPairSecond.second;
  if (flag1 && flag2) {
    if (leftLeaf != rightLeaf) {
      return wires[root] + " = " + inputs[rightLeaf] +
             typeFunc.find(root)->second + inputs[leftLeaf] + ";\n    ";
    } else {
      return wires[root] + " = ~" + inputs[rightLeaf] + ";\n    ";
    }
  } else if (!flag1 && !flag2) {
    if (leftLeaf != rightLeaf) {
      auto z = buildRIL(rightLeaf, cell, inputs, typeFunc, wires);
      auto z1 = buildRIL(leftLeaf, cell, inputs, typeFunc, wires);
      return z + z1 + wires[root] + " = " + wires[rightLeaf] + typeFunc[root] +
             wires[leftLeaf] + ";\n    ";
    } else {
      auto z = buildRIL(leftLeaf, cell, inputs, typeFunc, wires);
      return z + wires[root] + " = ~" + wires[leftLeaf] + ";\n    ";
    }
  } else if (!flag1 && flag2) {
    auto z = buildRIL(leftLeaf, cell, inputs, typeFunc, wires);
    return z + wires[root] + " = " + wires[leftLeaf] + typeFunc[root] +
           inputs[rightLeaf] + ";\n    ";
  } else {
    auto z = buildRIL(rightLeaf, cell, inputs, typeFunc, wires);
    return z + wires[root] + " = " + wires[rightLeaf] + typeFunc[root] +
           inputs[leftLeaf] + ";\n    ";
  }
}

void printCells(const YLib::dict<RTlil::IdString, RTlil::Cell *> &cells,
                std::map<int, std::string> &wires,
                std::map<std::string, int> &width,
                std::ofstream &fout,
                std::map<int, std::pair<int, int>> &cell,
                std::map<int, std::string> &typeFunc) {
  std::set<std::string> wireOutput;
  for (auto it1 = cells.begin(); it1 != cells.end(); ++it1) {
    size_t i = 0;
    bool flag = 0;
    std::string symbol;
    int a, b, c;
    for (auto it3 = it1->second->connections_.begin();
         it3 != it1->second->connections_.end(); ++it3) {
      i++;
      auto connAsWire = it3->second.as_wire()->name.index_;
      if (wires.find(connAsWire) != wires.end()) {
        wireOutput.insert(wires[connAsWire]);
      }
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
    }
  }
  for (auto it : wireOutput) {
    fout << "wire u:" << width[it] << " " << it << ";\n";
  }
}

void printConnections(
    const std::vector<std::pair<RTlil::SigSpec, RTlil::SigSpec>> &connections,
    std::ofstream &fout,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &typeFunc,
    std::map<int, std::string> &wires) {
  for (auto it1 = connections.begin(); it1 != connections.end(); ++it1) {
    auto connAsWireInput = it1->second.as_wire()->name.index_;
    auto connAsWireOutput = it1->first.as_wire()->name.index_;
    if (inputs.find(connAsWireInput) == inputs.end()) {
      fout << "@(*) {\n";
      fout << "    " << buildRIL(connAsWireInput, cell, inputs, typeFunc, wires)
           << outputs.find(connAsWireOutput)->second << " = "
           << wires[connAsWireInput] << ";\n";
      fout << "}\n";
    } else {
      fout << "@(*) {\n";
      fout << "   " << outputs.find(connAsWireOutput)->second << " = "
           << inputs.find(connAsWireInput)->second << ";\n";
      fout << "}\n";
    }
  }
}

void printSyncs(const std::vector<RTlil::SyncRule *> &syncs,
                std::ofstream &fout,
                std::map<int, std::string> &inputs,
                std::map<int, std::string> &outputs,
                std::map<int, std::string> &wires,
                std::string &state,
                int &temporary,
                std::map<int, std::pair<int, int>> &cell,
                std::map<int, std::string> &typeFunc,
                std::string &temp1,
                std::string &temp2,
                std::vector<std::string> &listSens,
                std::string &listFF) {
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
    }

    std::string inp = Yosys::log_signal((*it1)->signal);
    inp.erase(0, 1);
    listSens.push_back("@(" + state + "(" + inp + ")");
    if (state != "*")
      listFF = "@(" + state + "(" + inp + ")) {";
    else
      listFF = "@(*) {";

    for (auto it2 : (*it1)->actions) {
      std::string t1 = Yosys::log_signal((it2).first),
                  t2 = Yosys::log_signal((it2).second);
      if ((temp1 != t1) || (temp2 != t2)) {
        t1.erase(0, 1);
        t2.erase(0, 1);
        if (it2.second.is_wire()) {
          fout << "reg u:" << it2.second.as_wire()->width << " "
               << wires[it2.second.as_wire()->name.index_] << ";\n";
          fout << "@(*) {\n   " << t1 << " = "
               << wires[it2.second.as_wire()->name.index_] << ";\n}\n";
        }
      }
      temp1 = Yosys::log_signal((it2).first);
      temp2 = Yosys::log_signal((it2).second);
    }
  }
}

void printActions(const std::vector<RTlil::SigSig> &actions,
                  std::ofstream &fout, std::map<int, std::string> &inputs,
                  std::map<int, std::string> &outputs,
                  std::map<int, std::string> &wires,
                  std::map<int, std::pair<int, int>> &cell,
                  std::map<int, std::string> &typeFunc,
                  int &i,
                  bool &Flag,
                  std::string &func,
                  int &width,
                  bool &FF,
                  bool &DFF,
                  std::vector<std::string> &listSens,
                  std::string &cond,
                  std::string &listFF,
                  std::string &TMP_WIRES) {
  std::string tmp1 = "";
  int ll = 0;
  for (auto &it : actions) {
    int z = 0;
    ll++;
    for (auto itt : listSens) {
      tmp1 = Yosys::log_signal(it.first);
      bool cf = tmp1.find('{') != std::string::npos;
      if (DFF) {
        std::string tmp1 = Yosys::log_signal(it.first);
        bool cf = tmp1.find('{') != std::string::npos;
        if (!cf) {
          if (wires.find(it.first.as_wire()->name.index_) != wires.end()) {
            if (cell.find(it.second.as_wire()->name.index_) != cell.end()) {
              fout << "@(*) {\n";
              std::string st =
                  buildRIL(it.second.as_wire()->name.index_, cell, inputs,
                           typeFunc, wires)
                           .substr(0, buildRIL(it.second.as_wire()->name.index_,
                           cell, inputs, typeFunc, wires).size() - 4);
              fout << "    " << st;
              fout << "}\n";
            }
            fout << listFF << "\n    " << wires[it.first.as_wire()->name.index_]
                 << " = ";

          } else
            fout << listFF << "\n    "
                 << outputs[it.first.as_wire()->name.index_] << " = ";
          if (cell.find(it.second.as_wire()->name.index_) != cell.end()) {
            fout << wires[it.second.as_wire()->name.index_] << ";\n";
            fout << "}\n";
          } else if (inputs.find(it.second.as_wire()->name.index_) !=
                     inputs.end())
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
                    fout << "    " <<buildRIL(it.second.as_wire()->name.index_, cell,
                                   inputs, typeFunc, wires)
                       << "}\n";
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
              if (0 <= i && i <= listSens.size() - 1) {
                if (i == 0)
                  cond.erase(0, 1);
                fout << itt << ") "
                     << "if (" << cond << ") {\n";
                if (i == listSens.size() - 1) {
                  func += "~" + cond;
                }
                if (!it.second.is_wire()) {
                  std::string tmp = Yosys::log_signal(it.second);
//                  if (tmp == "1'0")
//                    tmp = "1b00000000";
//                  if (tmp == "1'1")
//                    tmp = "1b00000001";
                  fout << "    " << wires[it.first.as_wire()->name.index_]
                       << " = " << tmp << ";\n}\n";
                } else {
                  if (cell.find(it.second.as_wire()->name.index_) != cell.end())
                    fout << "    " << buildRIL(it.second.as_wire()->name.index_, cell,
                                     inputs, typeFunc, wires)
                         << wires[it.first.as_wire()->name.index_]
                         << " = " << wires[it.second.as_wire()->name.index_]

                         << ";\n}\n";
                  else
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = " << inputs[it.second.as_wire()->name.index_]
                         << ";\n}\n";
                }
              }
              if (i >= listSens.size() && i <= 2 * listSens.size() - 1) {
                // func += cond.erase(0, 1);
                //                if (i == listSens.size())
                //                  func.erase(func.size() - 1, func.size());
                z++;
                if (i == listSens.size()) {
                  fout << "reg u:" << width << " " << TMP_WIRES << ";\n";
                  fout << "@(*) {\n   " << TMP_WIRES << " = " << func
                       << ";\n}\n";
                }
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
                                     inputs, typeFunc, wires)
                         << "}\n";
                  else
                    fout << "    " << wires[it.first.as_wire()->name.index_]
                         << " = " << inputs[it.second.as_wire()->name.index_]
                         << ";\n}\n";
                }
                if (i == 2 * listSens.size() - 1)
                  TMP_WIRES += "1";
              }
              if (i >= 2 * listSens.size() && i <= 3 * listSens.size() - 1) {
                func.erase(func.size() - 2, func.size() - 1);
                func += "&(~" + cond + ")";
                fout << "reg u:" << width << " " << TMP_WIRES << ";\n";
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
                                     inputs, typeFunc, wires)
                         << "}\n";
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
      } else {
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
                    std::ofstream &fout,
                   std::map<int, std::string> &inputs,
                   std::map<int, std::string> &outputs,
                   std::map<int, std::string> &wires,
                   std::map<int, std::pair<int, int>> &cell,
                   std::map<int, std::string> &typeFunc,
                   int &i,
                   bool &Flag,
                   std::string &func,
                   int &width,
                   bool &FF,
                   bool &DFF,
                   std::vector<std::string> &listSens,
                   std::string &cond,
                   std::string &listFF,
                   std::string &TMP_WIRES) {
  for (auto &it : switches) {
    cond = Yosys::log_signal(it->signal);
    if (it->signal.is_wire())
      width = it->signal.as_wire()->width;
    for (auto it1 : it->cases) {
      printActions(it1->actions,
                   fout,
                   inputs,
                   outputs,
                   wires,
                   cell,
                   typeFunc,
                   i,
                   Flag,
                   func,
                   width,
                   FF,
                   DFF,
                   listSens,
                   cond,
                   listFF,
                   TMP_WIRES);
      printSwitches(it1->switches,
                    fout,
                    inputs,
                    outputs,
                    wires,
                    cell,
                    typeFunc,
                    i,
                    Flag,
                    func,
                    width,
                    FF,
                    DFF,
                    listSens,
                    cond,
                    listFF,
                    TMP_WIRES);
    }
  }
}
void printProcesses(
    const YLib::dict<RTlil::IdString, RTlil::Process *> &processes,
     std::ofstream &fout,
    std::map<int, std::string> &inputs,
    std::map<int, std::string> &outputs,
    std::map<int, std::string> &wires,
    std::map<int, std::pair<int, int>> &cell,
    std::map<int, std::string> &typeFunc,
    std::string &temp1,
    std::string &temp2,
    std::string &TMP_WIRES) {
  size_t pp = 0;
  bool FF = false;
  bool DFF = false;
  std::vector<std::string> listSens;
  int i = 0;
  bool Flag = false;
  std::string func = "";
  int width;
  std::string cond = "";
  std::string listFF = "";
  for (auto it1 = processes.begin(); it1 != processes.end(); ++it1) {
    pp++;
    std::string state;
    int temporary;
    auto rootCase = it1->second->root_case;
    printSyncs(it1->second->syncs,
               fout,
               inputs,
               outputs,
               wires,
               state,
               temporary,
               cell,
               typeFunc,
               temp1,
               temp2,
               listSens,
               listFF);
    if (rootCase.switches.empty()) {
      FF = 1;
    }
    printActions(rootCase.actions,
                 fout,
                 inputs,
                 outputs,
                 wires,
                 cell,
                 typeFunc,
                 i,
                 Flag,
                 func,
                 width,
                 FF,
                 DFF,
                 listSens,
                 cond,
                 listFF,
                 TMP_WIRES);
    printSwitches(rootCase.switches,
                  fout,
                  inputs,
                  outputs,
                  wires,
                  cell,
                  typeFunc,
                  i,
                  Flag,
                  func,
                  width,
                  FF,
                  DFF,
                  listSens,
                  cond,
                  listFF,
                  TMP_WIRES);
  }
}

void printParams(const std::pair<RTlil::IdString, RTlil::Module *> &m,
                 std::ofstream &fout) {
  std::map<int, std::string> inputs;
  std::map<int, std::string> outputs;
  std::map<int, std::string> typeFunc;
  std::map<int, std::string> wires;
  std::map<int, std::pair<int, int>> cell;
  std::map<std::string, int> width;
  std::string temp1 = "", temp2 = "";
  std::string TMP_WIRES = "_1";
  printWires(m.second->wires_,
             width,
             fout,
             inputs,
             outputs,
             wires,
             TMP_WIRES);
  printCells(m.second->cells_, wires, width, fout, cell, typeFunc);
  printConnections(m.second->connections_,
                   fout,
                   cell,
                   inputs,
                   outputs,
                   typeFunc,
                   wires);
  printProcesses(m.second->processes,
                 fout,
                 inputs,
                 outputs,
                 wires,
                 cell,
                 typeFunc,
                 temp1,
                 temp2,
                 TMP_WIRES);
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
    RTlil::Design *design = new RTlil::Design();
    Yosys::run_frontend(filename, "verilog", design, nullptr);
    size_t pos = filename.rfind(".");
    if (pos != std::string::npos) {
      filename.replace(pos, extension.length(), extension);
    }
    std::ofstream fout(filename);
    printParsed(*design, fout);
    fout.close();
    std::cout << "Parsed to: " << filename << "\n";
    std::unique_ptr<eda::rtl::model::Net> tttest =
        eda::rtl::parser::ril::parse(filename);
  }
  Yosys::yosys_shutdown();
}
