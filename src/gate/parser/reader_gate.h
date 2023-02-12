//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <fstream>
#include <iostream>

#include <lorina/verilog.hpp>

#include "gate/model/gnet.h"

class ReaderGate : public lorina::verilog_reader {
private:
  struct ParserData {
    struct GateData {
      std::vector<eda::gate::model::GNet::GateId> inputs;
      eda::gate::model::GNet::GateId id = 0;
      eda::gate::model::GateSymbol kind = eda::gate::model::GateSymbol::ZERO;
    };

    std::unordered_map<std::string, GateData> gates;
    std::unordered_map<eda::gate::model::GNet::GateId,
      eda::gate::model::GNet::GateId> gIds;
    // Wire name / <source, target>
    std::unordered_map<std::string, std::vector<std::string>> links;

    std::string netName;
    bool startParse = false;
    eda::gate::model::GNet gnet;
  };

  ParserData *data = new ParserData();

public:
  explicit ReaderGate(std::string name) {
    data->netName = std::move(name);
  }

  ~ReaderGate() {
    delete data;
  }

  eda::gate::model::GNet *getGnet() { return &data->gnet; }

  /*! \brief Callback method for parsed module.
   *
   * \param moduleName Name of the module
   * \param inouts Container for input and output names
   */
  void on_module_header(const std::string &moduleName,
                        const std::vector<std::string> &inputs) const override {
    data->startParse = moduleName == data->netName;
  }

  /*! \brief Callback method for parsed inputs.
   *
   * \param inputs Input names
   * \param size Size modifier
   */
  void on_inputs(const std::vector<std::string> &inputs,
                 std::string const &size) const override {
    if (data->startParse) {
      for (const std::string &input: inputs) {
        std::string in = "#" + input;
        auto &gate = data->gates[in];
        gate.id = data->gnet.newGate();
        data->gIds.emplace(gate.id, data->gates.size() - 1);
        gate.kind = eda::gate::model::GateSymbol::IN;
        data->links[input] = {in, ""};
      }
    }
  }

  /*! \brief Callback method for parsed wires.
   *
   * \param wires Wire names
   * \param size Size modifier
   */
  void on_wires(const std::vector<std::string> &wires,
                std::string const &size) const override {
    if (data->startParse) {
      for (auto &name: wires) {
        auto &gates = data->links[name];
        gates.reserve(3);
        gates.resize(1);
      }
    }
  }

  /*! \brief Callback method for parsed module instantiation of form `NAME
   * #(P1,P2) NAME(.SIGNAL(SIGANL), ..., .SIGNAL(SIGNAL));`
   *
   * \param moduleName Name of the module
   * \param params List of parameters
   * \param instName Name of the instantiation
   * \param args List (a_1,b_1), ..., (a_n,b_n) of name pairs, where
   *             a_i is a name of a signals in moduleName and b_i is a name of
   * a signal in instName.
   */
  void on_module_instantiation(
    std::string const &moduleName, std::vector<std::string> const &params,
    std::string const &instName,
    std::vector<std::pair<std::string, std::string>> const &args) const override {
    if (data->startParse) {
      auto &gateData = data->gates[instName];
      gateData.id = data->gnet.newGate();
      data->gIds.emplace(gateData.id, data->gates.size() - 1);
      gateData.kind = symbol(moduleName);

      insertLink(args[0].second, instName, false);
      for (size_t i = 1; i < args.size(); ++i) {
        insertLink(args[i].second, instName, true);
      }
    }
  }

  /*! \brief Callback method for parsed endmodule.
   *
   */
  void on_endmodule() const override {
    if (data->startParse) {
      //  Collect links to make inputs arrays.
      for (auto &[name, links]: data->links) {
        auto source = data->gates.find(links[0]);

        for (size_t i = 1; i < links.size(); ++i) {
          auto target = data->gates.find(links[i]);
          if (source != data->gates.end() && target != data->gates.end()) {
            target->second.inputs.push_back(source->second.id);
          }
        }

      }
      //  All gates are created - modifying them.
      for (const auto &[name, gateData]: data->gates) {
        std::vector<eda::base::model::Signal < eda::gate::model::GNet::GateId>>
        signals;
        signals.reserve(gateData.inputs.size());

        for (auto input: gateData.inputs) {
          signals.emplace_back(eda::base::model::Event::ALWAYS, input);
        }

        data->gnet.setGate(gateData.id, gateData.kind, signals);
      }
    }
  }

  void print() const {
    for (auto &gate: data->gnet.gates()) {
      std::cout << gate->id() << " :\n"; // << " " << gate->kind()
      for (auto &link: gate->links()) {
        std::cout << "\t( " << link.source << " ) " << link.target << "\n";
      }
    }
  }

  void static print(std::ofstream &stream, const eda::gate::model::Gate *gate) {
    stream << gate->id(); // << " " << gate->kind()
  }

  void dotPrint(const std::string &filename) const {
    std::ofstream out(filename);
    dot(out);
    out.close();
  }

  void dot(std::ofstream &stream) const {
    stream << "digraph gnet {\n";
    for (const auto &gate: data->gnet.gates()) {
      for (auto &links: gate->links()) {
        stream << "\t";
        print(stream, gate);
        stream << " -> ";
        print(stream, data->gnet.gate(data->gIds.at(links.target)));
        stream << ";\n";
      }
    }
    stream << "}" << std::endl;
  }

private:
  void insertLink(const std::string &name, const std::string &instName,
                  bool out) const {
    auto &link = data->links[name];
    if (out) {
      link.emplace_back(instName);
    } else {
      assert(link[0].empty());
      link[0] = instName;
    }
  }

  static eda::gate::model::GateSymbol symbol(const std::string &s) {
    if (s == "not") {
      return eda::gate::model::GateSymbol::NOT;
    } else if (s == "or") {
      return eda::gate::model::GateSymbol::OR;
    } else if (s == "xor") {
      return eda::gate::model::GateSymbol::XOR;
    } else if (s == "nand") {
      return eda::gate::model::GateSymbol::NAND;
    } else if (s == "nor") {
      return eda::gate::model::GateSymbol::NOR;
    } else if (s == "xnor") {
      return eda::gate::model::GateSymbol::XNOR;
    } else if (s == "and") {
      return eda::gate::model::GateSymbol::AND;
    }
    return eda::gate::model::GateSymbol::ZERO;
  }
};

