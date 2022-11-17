//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <iostream>

#include "rtl/model/pnode.h"
#include "rtl/model/vnode.h"

namespace eda::rtl::model {

std::ostream &operator <<(std::ostream &out, const PNode &pnode) {
  out << "always @(" << pnode.signal() << ") ";

  if (pnode.gsize() > 0) {  
    out << "if (";
    bool separator = false;
    for (const auto *vnode: pnode.guard()) {
      out << (separator ? " && " : "") << *vnode;
      separator = true;
    }
    out << ") {" << std::endl;
  } else {
    out << "{" << std::endl;
  }

  for (const auto *vnode: pnode.action()) {
    out << "  " << *vnode << std::endl;
  }

  out << "}";
  return out;
}

} // namespace eda::rtl::model
