//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

#include <memory>

///defines the function that parses the bench file. 
std::unique_ptr<eda::gate::model::GNet> parseBenchFile(const std::string &filename);
