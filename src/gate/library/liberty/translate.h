//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/library/liberty/net_data.h"
#include "gate/model/gnet.h"

#include <kernel/yosys.h>

#include <string>

bool translateModuleToGNet(
    const Yosys::RTLIL::Module *m,
    eda::gate::model::GNet &net);

void translateDesignToGNet(
    const Yosys::RTLIL::Design *des,
    NetData &vec);

void translateLibertyToDesign(
    const std::string namefile,
    NetData &vec);
