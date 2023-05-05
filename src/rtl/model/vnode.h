//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "base/model/link.h"
#include "base/model/node.h"
#include "base/model/signal.h"
#include "rtl/model/fsymbol.h"
#include "rtl/model/variable.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace eda::rtl::model {

class Net;
class PNode;

using VNodeBase = eda::base::model::Node<FuncSymbol>;

/**
 * \brief Represents a V-node (V = variable), which is a functional or
 *        communication unit of the RTL design.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class VNode final : public VNodeBase {
  // Creating a node.
  friend class Net;
  // Setting the parent P-node.
  friend class PNode;

public:
  using List = std::vector<VNode*>;

  enum Kind {
    /// Source node (S-node):
    ///   input wire x.
    SRC,
    /// Constant node (C-node):
    ///   y <= (c[0], ..., c[n-1]).
    VAL,
    /// Functional node (F-node):
    ///   always_comb y <= f(x[0], ..., x[n-1]).
    FUN,
    /// Multiplexor node (M-node):
    ///   always_comb y <= mux(x[0], ..., x[n-1]).
    MUX,
    /// Register node (R-node):
    ///   always_ff @(edge) y <= x or always_latch if(level) y <= x.
    REG
  };

  /// Returns the node w/ the given id from the storage.
  static VNode *get(Id id) { return static_cast<VNode*>(VNodeBase::get(id)); }

  Kind kind() const { return _kind; }

  const Variable &var() const { return _var; }
  const std::string &name() const { return _var.name(); }
  const Type &type() const { return _var.type(); }
  size_t width() const { return type().width(); }

  bool isOutput() const { return _var.bind() == Variable::OUTPUT; }

  const std::vector<bool> value() const { return _value; }

  const PNode *pnode() const { return _pnode; }

private:
  VNode(Kind kind,
        const Variable &var,
        FuncSymbol func,
        const SignalList &inputs,
        const std::vector<bool> &value):
      VNodeBase(func, inputs),
      _kind(kind),
      _var(var),
      _value(value),
      _pnode(nullptr) {
    assert(std::find(inputs.begin(), inputs.end(), VNode::INVALID) == inputs.end());
  }

  VNode(Id id,
        Kind kind,
        const Variable &var,
        FuncSymbol func,
        const SignalList &inputs,
        const std::vector<bool> &value,
        const LinkList &links):
      VNodeBase(id, func, inputs, links),
      _kind(kind),
      _var(var),
      _value(value),
      _pnode(nullptr) {
    assert(std::find(inputs.begin(), inputs.end(), VNode::INVALID) == inputs.end());
  }

  VNode *duplicate(const std::string &newName) {
    Variable var(newName, _var.kind(), _var.bind(), _var.type());
    return new VNode(_kind, var, _func, _inputs, _value);
  }

  void replaceWith(Kind kind,
                   const Variable &var,
                   FuncSymbol func,
                   const SignalList &inputs,
                   const std::vector<bool> &value) {
    // Save the identifier and the links.
    Id oldId = _id;
    LinkList oldLinks = _links;
    // Disconnect from the drivers.
    this->setInputs({});
    // Replace the node w/ a new one.
    this->~VNode();
    new (this) VNode(oldId, kind, var, func, inputs, value, oldLinks);
  }

  void setPNode(const PNode *pnode) {
    assert(pnode != nullptr);
    _pnode = pnode;
  }

  const Kind _kind;
  const Variable _var;
  const std::vector<bool> _value;

  // Parent P-node (set on P-node creation).
  const PNode *_pnode;
};

std::ostream& operator <<(std::ostream &out, const VNode &vnode);

} // namespace eda::rtl::model
