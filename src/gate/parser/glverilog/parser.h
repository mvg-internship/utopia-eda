//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once
#include <gate/model/gate.h>
#include <memory>
#include <string>
#include <vector>



bool parseGateLevelVerilog(const std::string &path, 
                           std::vector<std::unique_ptr<eda::gate::model::GNet>> &nets);
