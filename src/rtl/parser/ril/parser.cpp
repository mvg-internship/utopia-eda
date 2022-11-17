//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <stdio.h>

#include "rtl/model/net.h"
#include "rtl/parser/ril/builder.h"
#include "rtl/parser/ril/parser.h"

// The parser is built w/ the prefix 'rr' (not 'yy').
extern FILE *rrin;
extern int rrparse(void);

namespace eda::rtl::parser::ril {

std::unique_ptr<eda::rtl::model::Net> parse(const std::string &filename) {
  FILE *file = fopen(filename.c_str(), "r");
  if (file == nullptr) {
    return nullptr;
  }

  rrin = file;
  if (rrparse() == -1) {
    return nullptr;
  }

  std::unique_ptr<Net> net = Builder::get().create();
  if (net == nullptr) {
    return nullptr;
  }

  net->create();
  return net;
}

} // namespace eda::rtl::parser::ril
