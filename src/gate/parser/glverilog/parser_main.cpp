//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "header_file"
#include "parser.h"
#include "token.h"

#include <algorithm>
#include <fstream>
#include <gate/model/gate.h>
#include <gate/model/gnet.h>
#include <gate/model/gsymbol.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;

int main(int argc, char *argv[]) {
    std::vector<std::unique_ptr<GNet>> nets;
    bool success = parseGateLevelVerilog(argv[1], nets);
    if (success) {
        std::cout << "Parsing successful" << std::endl;
        for (const auto &gnet_ptr : nets) {
            std::cout << "GNet's: " << std::endl << *gnet_ptr << std::endl;
        }
    } else {
        std::cout << "Parsing failed" << std::endl;
    }
    return success ? 0 : 1;
}