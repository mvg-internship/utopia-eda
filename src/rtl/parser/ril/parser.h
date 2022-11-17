//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>

namespace eda::rtl::model {
  class Net;
} // namespace eda::rtl::model

namespace eda::rtl::parser::ril {

std::unique_ptr<eda::rtl::model::Net> parse(const std::string &filename);

} // namespace eda::rtl::parser::ril
