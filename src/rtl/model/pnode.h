//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include <iostream>
#include <vector>

#include "base/model/signal.h"
#include "rtl/model/vnode.h"

namespace eda::rtl::model {

/**
 * \brief Represents a p-node (p = process), a guarded action.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class PNode final {
  // Creation.
  friend class Net;

public:
  using List = std::vector<PNode*>;
  using Signal = VNode::Signal;
  using SignalList = VNode::SignalList;

  const Signal &signal() const { return _signal; }

  size_t gsize() const { return _guard.size(); }
  const VNode::List &guard() const { return _guard; }
  const VNode *guard(std::size_t i) const { return _guard[i]; }

  size_t asize() const { return _action.size(); }
  const VNode::List &action() const { return _action; }
  const VNode* action(std::size_t i) const { return _action[i]; }

private:
  PNode(const Signal &signal, const VNode::List &guard, const VNode::List &action):
      _signal(signal), _guard(guard), _action(action) {
    for (auto *vnode: guard) {
      vnode->setPNode(this);
    }
    for (auto *vnode: action) {
      vnode->setPNode(this);
    }
  }

  PNode(const VNode::List &guard, const VNode::List &action):
      PNode(Signal::always(VNode::INVALID), guard, action) {}

  /// The execution trigger (posedge, always, etc.).
  const Signal _signal;
  /// The last v-node is the guard bit.
  VNode::List _guard;
  /// The non-blocking assignments.
  VNode::List _action;
};

std::ostream &operator <<(std::ostream &out, const PNode &pnode);

} // namespace eda::rtl::model
