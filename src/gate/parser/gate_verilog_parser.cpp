//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/parser/gate_verilog_parser.h"

using GateId = eda::gate::model::GNet::GateId;
using GateSymbol = eda::gate::model::GateSymbol;

GateVerilogParser::GateVerilogParser(std::string name) {
  data->netName = std::move(name);
}

GateVerilogParser::~GateVerilogParser() {
  delete data;
}

eda::gate::model::GNet *GateVerilogParser::getGnet() { return data->gnet; }

void GateVerilogParser::on_module_header(const std::string &moduleName,
                                         const std::vector<std::string> &inputs)
const {
  data->startParse = moduleName == data->netName;
}

void GateVerilogParser::on_inputs(const std::vector<std::string> &inputs,
                                  std::string const &size) const {
  if (data->startParse) {
    for (
      const std::string &input: inputs) {
      std::string in = "#" + input;
      auto &gate = data->gates[in];
      gate.id = data->gnet->newGate();
      data->gIds.emplace(gate.id, data->gates.size() - 1);
      gate.kind = GateSymbol::IN;
      data->links[input] = {in, ""};
    }
  }
}

void GateVerilogParser::on_wires(const std::vector<std::string> &wires,
                                 std::string const &size) const {
  if (data->startParse) {
    for (const auto &name: wires) {
      auto &gates = data->links[name];
      gates.reserve(3);
      gates.resize(1);
    }
  }
}

void GateVerilogParser::on_module_instantiation(
        std::string const &moduleName, std::vector<std::string> const &params,
        std::string const &instName,
        std::vector<std::pair<std::string, std::string>> const &args) const {
  if (data->startParse) {
    auto &gateData = data->gates[instName];
    gateData.id = data->gnet->newGate();

    data->gIds.emplace(gateData.id, data->gates.size() - 1);
    gateData.kind = symbol(moduleName);

    insertLink(args[0].second, instName, false);
    for (size_t i = 1; i < args.size(); ++i) {
      insertLink(args[i].second, instName, true);
    }
  }
}

void GateVerilogParser::on_endmodule() const {
  if (data->startParse) {
    // Collect links to make inputs arrays.
    for (const auto &[name, links]: data->links) {
      auto source = data->gates.find(links[0]);

      for (size_t i = 1; i < links.size(); ++i) {
        auto target = data->gates.find(links[i]);
        if (source != data->gates.end() && target != data->gates.end()) {
          target->second.inputs.push_back(source->second.id);
        }
      }

    }
    // All gates are created - modifying them.
    for (const auto &[name, gateData]: data->gates) {
      std::vector<eda::base::model::Signal<GateId>> signals;
      signals.reserve(gateData.inputs.size());

      for (auto input: gateData.inputs) {
        signals.emplace_back(eda::base::model::Event::ALWAYS, input);
      }

      data->gnet->setGate(gateData.id, gateData.kind, signals);
    }
  }
}

void GateVerilogParser::insertLink(const std::string &name,
                                   const std::string &instName,
                                   bool out) const {
  auto &link = data->links[name];
  if (out) {
    link.emplace_back(instName);
  } else {
    assert(link[0].empty());
    link[0] = instName;
  }
}

GateSymbol GateVerilogParser::symbol(const std::string &s) const {
  if (s == "not") {
    return GateSymbol::NOT;
  } else if (s == "or") {
    return GateSymbol::OR;
  } else if (s == "xor") {
    return GateSymbol::XOR;
  } else if (s == "nand") {
    return GateSymbol::NAND;
  } else if (s == "nor") {
    return GateSymbol::NOR;
  } else if (s == "xnor") {
    return GateSymbol::XNOR;
  } else if (s == "and") {
    return GateSymbol::AND;
  }
  return GateSymbol::ZERO;
}
