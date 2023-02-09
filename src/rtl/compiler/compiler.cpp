//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "rtl/compiler/compiler.h"
#include "rtl/library/flibrary.h"
#include "rtl/model/net.h"
#include "rtl/model/vnode.h"
#include "util/string.h"

#include <cassert>
#include <cstddef>
#include <memory>

using namespace eda::gate::model;
using namespace eda::rtl::library;
using namespace eda::rtl::model;
using namespace eda::utils;

namespace eda::rtl::compiler {

std::unique_ptr<GNet> Compiler::compile(const Net &net) {
  // Create a new gate-level net.
  auto gnet = std::make_unique<GNet>();

  // Initialize correspondence between vnodes and gates.
  _outputs.clear();

  // To cut through the cycles, synthesis of registers is postponed.
  VNode::List registers;
  registers.reserve(net.vsize());

  // It is assumed that vnodes are topologically sorted.
  for (auto *vnode: net.vnodes()) {
    switch (vnode->kind()) {
    case VNode::SRC:
      synthSrc(vnode, *gnet);
      break;
    case VNode::VAL:
      synthVal(vnode, *gnet);
      break;
    case VNode::FUN:
      synthFun(vnode, *gnet);
      break;
    case VNode::MUX:
      synthMux(vnode, *gnet);
      break;
    case VNode::REG:
      allocReg(vnode, *gnet);
      registers.push_back(vnode);
      break;
    }

    if (vnode->isOutput()) {
      // Allocate output pseudo gates.
      synthOut(vnode, *gnet);
    }
  }

  // Synthesize gates for the postponed registers.
  for (auto *vnode: registers) {
    synthReg(vnode, *gnet);
  }

  return gnet;
}

void Compiler::synthSrc(const VNode *vnode, GNet &net) {
  auto out = _library.alloc(vnode->width(), net);
  _outputs.insert({vnode->id(), out});
}

void Compiler::synthVal(const VNode *vnode, GNet &net) {
  auto out = _library.synth(vnode->width(), vnode->value(), net);
  _outputs.insert({vnode->id(), out});
}

void Compiler::synthOut(const VNode *vnode, GNet &net) {
  _library.synth(vnode->width(), out(vnode), net);
}

void Compiler::synthFun(const VNode *vnode, GNet &net) {
  assert(_library.supports(vnode->func()));
  auto out = _library.synth(vnode->width(), vnode->func(), in(vnode), net);
  _outputs.insert({vnode->id(), out});
}

void Compiler::synthMux(const VNode *vnode, GNet &net) {
  assert(_library.supports(FuncSymbol::MUX));
  auto out = _library.synth(vnode->width(), FuncSymbol::MUX, in(vnode), net);
  _outputs.insert({vnode->id(), out});
}

void Compiler::allocReg(const VNode *vnode, GNet &net) {
  auto out = _library.alloc(vnode->width(), net);
  _outputs.insert({vnode->id(), out});
}

void Compiler::synthReg(const VNode *vnode, GNet &net) {
  // Level (latch), edge (flip-flop), or edge and level (flip-flop /w set/reset).
  assert(vnode->nSignals() == 1 || vnode->nSignals() == 2);

  GNet::SignalList control;
  for (const auto &signal: vnode->signals()) {
    const auto &signalOut = out(signal.node());
    assert(signalOut.size() == 1);
    control.push_back(GNet::Signal(signal.event(), signalOut[0]));
  }

  _library.synth(out(vnode), in(vnode), control, net);
}

GNet::In Compiler::in(const VNode *vnode) const {
  GNet::In in(vnode->arity());
  for (size_t i = 0; i < vnode->arity(); i++) {
    in[i] = out(vnode->input(i).node());
  }

  return in;
}

const GNet::Out &Compiler::out(const VNode *vnode) const {
  return out(vnode->id());
}

const GNet::Out &Compiler::out(VNode::Id vnodeId) const {
  auto i = _outputs.find(vnodeId);
  assert(i != _outputs.end());
  return i->second;
}

} // namespace eda::rtl::compiler
