//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <iostream>

#include "rtl/model/vnode.h"

namespace eda::rtl::model {

static std::ostream &operator <<(std::ostream &out, const std::vector<bool> &value) {
  for (bool bit: value) {
    out << bit;
  }

  return out;
}

static std::ostream &operator <<(std::ostream &out, const VNode::SignalList &signals) {
  bool separator = false;
  for (const auto &signal : signals) {
    const auto *vnode = VNode::get(signal.node());
    out << (separator ? ", " : "") << vnode->name();
    separator = true;
  }

  return out;
}

std::ostream &operator <<(std::ostream &out, const VNode &vnode) {
  out << vnode.id() << ": ";
  switch (vnode.kind()) {
  case VNode::SRC:
    return out << "S{" << vnode.var() << "}";
  case VNode::VAL:
    return out << "C{" << vnode.var() << " = "
                       << vnode.value()<< "}";
  case VNode::FUN:
    return out << "F{" << vnode.var() << " = "
                       << vnode.func() << "(" << vnode.inputs() << ")}";
  case VNode::MUX:
    return out << "M{" << vnode.var() << " = "
                       << "mux(" << vnode.inputs() << ")}";
  case VNode::REG:
    out << "R{";
    bool separator = false;
    for (std::size_t i = 0; i < vnode.arity(); i++) {
      out << (separator ? ", " : "");
      if (i < vnode.nSignals()) {
        out << vnode.signal(i) << ": ";
      }
      const auto *rhs = VNode::get(vnode.input(i).node());
      out << vnode.var() << " = " << rhs->name();
      separator = true;
    }
    return out << "}";
  }

  return out;
}

} // namespace eda::rtl::model
