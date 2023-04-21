//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "rtl/model/net.h"
#include "util/graph.h"
#include "util/string.h"

#include <cassert>
#include <cstddef>

using namespace eda::utils;

namespace eda::rtl::model {
 
void Net::create() {
  // Net cannot be created multiple times.
  assert(!_isCreated);

  for (auto &[_, usage]: _vnodesTemp) {
    assert(!_.empty());

    VNode *phi = usage.first;
    VNode::List &defines = usage.second;

    // Multiple definitions <=> phi-node is required.
    assert((phi != nullptr && defines.size() >= 2) ||
           (phi == nullptr && defines.size() == 1));

    // For registers, the node is updated even for a single definition:
    // it is supplemented w/ the signal triggering the parent P-node.
    phi = (phi != nullptr ? phi : defines.front());

    switch (phi->var().kind()) {
    case Variable::WIRE:
      muxWireDefines(phi, defines); // Updates _vnodes.
      break;
    case Variable::REG:
      muxRegDefines(phi, defines);  // Updates _vnodes.
      break;
    }
  }

  // Release useless nodes.
  applyRelease();

  _vnodesTemp.clear();
  _pnodes.clear();

  sortTopologically();
  _isCreated = true;
}

// if (g[1]) { w <= f[1](...) }    w[1] <= f[1](...)
// ...                          => ...               + w <= mux{ g[i] -> w[i] }
// if (g[n]) { w <= f[n](...) }    w[n] <= f[n](...)
void Net::muxWireDefines(VNode *phi, const VNode::List &defines) {
  const size_t n = defines.size();
  assert(n != 0);

  // No multiplexing is required.
  if (n == 1) {
    VNode *vnode = defines.front();
    addVNodeFinal(vnode);
    return;
  }

  // Create the { w[i] } nodes and compose the mux inputs: { g[i] -> w[i] }.
  SignalList inputs(2 * n);

  for (size_t i = 0; i < n; i++) {
    VNode *oldVNode = defines[i];

    assert(oldVNode->pnode() != nullptr);
    assert(oldVNode->pnode()->gsize() > 0);

    // Create a { w[i] <= f[i](...) } node.
    VNode *newVNode = oldVNode->duplicate(unique_name(oldVNode->name()));
    addVNodeFinal(newVNode);

    // Guards come first: mux(g[1], ..., g[n]; w[1], ..., w[n]).
    inputs[i] = oldVNode->pnode()->guard().back()->always();
    inputs[i + n] = newVNode->always();

    scheduleRelease(oldVNode);
  }

  // Connect the wire w/ the multiplexor: w <= mux{ g[i] -> w[i] }.
  phi->replaceWith(VNode::MUX, phi->var(), FuncSymbol::NOP, inputs, {});
  addVNodeFinal(phi);
}

// @(signal): if (g[1]) { r <= w[1] }    w <= mux{ g[i] -> w[i] }
// ...                     =>
// @(signal): if (g[n]) { r <= w[n] }    @(signal): r <= w
void Net::muxRegDefines(VNode *phi, const VNode::List &defines) {
  std::vector<std::pair<Signal, VNode::List>> groups = groupRegDefines(defines);

  const size_t n = groups.size();
  assert(n != 0);

  Variable output = phi->var();

  // Control signals c[1], ..., c[n] and data signals d[1], ..., d[n].
  SignalList inputs(2 * n);

  size_t i = 0;
  for (const auto &[signal, defines]: groups) {
    // Create a wire w for the given signal.
    const VNode *vnode = VNode::get(signal.node());
    const std::string name = output.name() + "$" + vnode->name();
    Variable wire(name, Variable::WIRE, output.type());

    // Create a multiplexor: w <= mux{ g[i] -> w[i] }.
    VNode *mux = createMux(wire, defines);
    addVNodeFinal(mux);

    inputs[i] = signal;
    inputs[i + n] = mux->always();
    i++;
  }

  // Connect the register w/ the multiplexor(s) via the wire(s): r <= w.
  phi->replaceWith(VNode::REG, output, FuncSymbol::NOP, inputs, {});
  addVNodeFinal(phi);
}

std::vector<std::pair<VNode::Signal, VNode::List>> Net::groupRegDefines(const VNode::List &defines) {
  const Signal *clock = nullptr;
  const Signal *level = nullptr;

  VNode::List clockDefines;
  VNode::List levelDefines;

  // Collect all the signals triggering the register.
  for (VNode *vnode: defines) {
    assert(vnode != nullptr && vnode->pnode() != nullptr);

    const Signal &signal = vnode->pnode()->signal();
    assert(signal.isEdge() || signal.isLevel());

    if (signal.isEdge()) {
      // At most one edge-triggered signal (clock) is allowed.
      assert(clock == nullptr || *clock == signal);
      clock = &signal;
      clockDefines.push_back(vnode);
    } else {
      // At most one level-triggered signal (enable or reset) is allowed.
      assert(level == nullptr || *level == signal);
      level = &signal;
      levelDefines.push_back(vnode);
    }
  }

  std::vector<std::pair<Signal, VNode::List>> groups;
  if (clock != nullptr) {
    groups.push_back({ *clock, clockDefines });
  }
  if (level != nullptr) {
    groups.push_back({ *level, levelDefines });
  }

  return groups;
}

VNode *Net::createMux(const Variable &output, const VNode::List &defines) {
  const size_t n = defines.size();
  assert(n != 0);

  // Multiplexor is not required.
  if (n == 1) {
    VNode *vnode = defines.front();
    SignalList inputs{vnode->input(0)};

    scheduleRelease(vnode);
    return new VNode(VNode::FUN, output, FuncSymbol::NOP, inputs, {}); 
  }

  // Compose the mux inputs { g[i] -> w[i] }.
  SignalList inputs(2 * n);

  for (size_t i = 0; i < n; i++) {
    VNode *vnode = defines[i];
    assert(vnode->pnode() != nullptr);

    // Guards come first: mux(g[1], ..., g[n]; w[1], ..., w[n]).
    inputs[i] = vnode->pnode()->guard().back()->always();
    inputs[i + n] = vnode->input(0);

    scheduleRelease(vnode);
  }

  // Create a multiplexor: w <= mux{ g[i] -> w[i] }.
  return new VNode(VNode::MUX, output, FuncSymbol::NOP, inputs, {});
}

void Net::sortTopologically() {
  assert(!_isCreated);

  auto vnodeIds = eda::utils::graph::topologicalSort<Net>(*this);
  for (size_t i = 0; i < vnodeIds.size(); i++) {
    auto vnodeId = vnodeIds[i];
    _vnodes[i] = VNode::get(vnodeId);
  }
}

std::ostream &operator <<(std::ostream &out, const Net &net) {
  for (const auto *pnode: net.pnodes()) {
    out << *pnode << std::endl;
  }

  for (const auto *vnode: net.vnodes()) {
    out << *vnode << std::endl;
  }

  return out;
}

} // namespace eda::rtl::model
