//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/printer/dot.h"

std::vector<std::string> Dot::funcNames = [] {
  std::vector<std::string> names;
  names.reserve(GateSymbol::XXX + 1);

  auto insert = [&names](GateSymbol gs, std::string &&name) {
    if (names.size() <= gs) {
      names.resize(gs + 1);
      names[gs] = std::move(name);
    }
  };

  insert(GateSymbol::IN, "IN");
  insert(GateSymbol::OUT, "OUT");
  insert(GateSymbol::ZERO, "ZERO");
  insert(GateSymbol::ONE, "ONE");
  insert(GateSymbol::NOP, "NOP");
  insert(GateSymbol::NOT, "NOT");
  insert(GateSymbol::AND, "AND");
  insert(GateSymbol::OR, "OR");
  insert(GateSymbol::XOR, "XOR");
  insert(GateSymbol::NAND, "NAND");
  insert(GateSymbol::NOR, "NOR");
  insert(GateSymbol::XNOR, "XNOR");
  insert(GateSymbol::MAJ, "MAJ");
  insert(GateSymbol::LATCH, "LATCH");
  insert(GateSymbol::DFF, "DFF");
  insert(GateSymbol::DFFrs, "DFFrs");
  insert(GateSymbol::XXX, "XXX");
  return names;
}();

Dot::Dot(const Dot::GNet *gNet) : gNet(gNet) {}

void Dot::print(const std::string &filename) const {
  std::ofstream out(filename);
  if (out.is_open()) {
    print(out);
    out.close();
  } else {
    std::cerr << "Failed to create file : " << filename << std::endl;
  }
}

void Dot::print(std::ofstream &stream) const {
  stream << "digraph subNet {\n";
  for (const auto &gate: gNet->gates()) {
    if (gate->links().empty()) {
      stream << "\t";
      print(stream, gate);
      stream << ";\n";
    }
    for (const auto &links: gate->links()) {
      stream << "\t";
      print(stream, gate);
      stream << " -> ";
      print(stream, Gate::get(links.target));
      stream << ";\n";
    }
  }
  stream << "}" << std::endl;
}

void
Dot::print(std::ofstream &stream, const eda::gate::model::Gate *gate) const {
  if(funcNames.size() > gate->func()) {
    stream << funcNames[gate->func()];
  }
  stream << gate->id();
}
