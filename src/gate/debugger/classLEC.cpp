//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "classLEC.h"
#include "checker.h"
#include "classRND.h"

namespace eda::gate::debugger {
LEC &getChecker(LecType lec) {
  switch(lec) {
    case LecType::DFL: return Checker::get();
    case LecType::RND: return RndChecker::get();
    default: return Checker::get();
  }
}
LEC::~LEC() {};

} // namespace eda::gate::debugger
