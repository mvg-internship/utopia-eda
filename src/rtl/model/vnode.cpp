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
    out << "S{" << vnode.var() << "}";
    break;
  case VNode::VAL:
    out << "C{" << vnode.var() << " = "
                << vnode.value() << "}";
    break;
  case VNode::FUN:
    out << "F{" << vnode.var() << " = "
                << vnode.func() << "(" << vnode.inputs() << ")}";
    break;
  case VNode::MUX:
    out << "M{" << vnode.var() << " = "
                << "mux(" << vnode.inputs() << ")}";
    break;
  case VNode::REG:
    out << "R{" << vnode.var() << " = "
                << "reg(" << vnode.inputs() << ")}";
    break;
  }

  out << "[fo=" << vnode.fanout() << "]";
  return out;
}

} // namespace eda::rtl::model
