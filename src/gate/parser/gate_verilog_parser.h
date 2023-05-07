//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

#include <lorina/verilog.hpp>

#include <fstream>
#include <iostream>

/**
 * \brief Verilog parser based on Lorina.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
class GateVerilogParser : public lorina::verilog_reader {
private:
  struct ParserData {
    using GateId = eda::gate::model::GNet::GateId;
    using GateSymbol = eda::gate::model::GateSymbol;
    struct GateData {
      std::vector<GateId> inputs;
      GateId id = 0;
      GateSymbol kind = GateSymbol::ZERO;
    };

    std::unordered_map<std::string, GateData> gates;
    std::unordered_map<GateId,GateId> gIds;
    // Wire name / <source, target>
    std::unordered_map<std::string, std::vector<std::string>> links;

    std::string netName;
    bool startParse = false;
    eda::gate::model::GNet *gnet = new eda::gate::model::GNet();
  };

  ParserData *data = new ParserData();

public:
  explicit GateVerilogParser(std::string name);

  ~GateVerilogParser();

  eda::gate::model::GNet *getGnet();

  /*! \brief Callback method for parsed module.
   *
   * \param moduleName Name of the module
   * \param inouts Container for input and output names
   */
  void on_module_header(const std::string &moduleName,
                        const std::vector<std::string> &inputs) const override;

  /*! \brief Callback method for parsed inputs.
   *
   * \param inputs Input names
   * \param size Size modifier
   */
  void on_inputs(const std::vector<std::string> &inputs,
                 std::string const &size) const override;

  /*! \brief Callback method for parsed wires.
   *
   * \param wires Wire names
   * \param size Size modifier
   */
  void on_wires(const std::vector<std::string> &wires,
                std::string const &size) const override;

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
          std::vector<std::pair<std::string, std::string>> const &args) const override;

  /*! \brief Callback method for parsed endmodule.
   *
   */
  void on_endmodule() const override;

private:
  void insertLink(const std::string &name, const std::string &instName,
                  bool out) const;

  ParserData::GateSymbol symbol(const std::string &s) const;
};
