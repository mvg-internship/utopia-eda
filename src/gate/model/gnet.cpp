//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "util/graph.h"
#include "util/set.h"

#include <algorithm>
#include <cassert>
#include <queue>

using namespace eda::utils;
using namespace eda::utils::graph;

namespace eda::gate::model {

//===----------------------------------------------------------------------===//
// Constructors/Destructors
//===----------------------------------------------------------------------===//

unsigned GNet::_counter = 0;

GNet::GNet(unsigned level):
    _id(_counter++),
    _level(level),
    _nConnects(0),
    _nGatesInSubnets(0),
    _isSorted(true) {
  const size_t N = 1024;
  const size_t M = 64;

  _gates.reserve(N);
  _flags.reserve(N);

  _sourceLinks.reserve(M);
  _targetLinks.reserve(M);

  _subnets.reserve(M);
}

//===----------------------------------------------------------------------===//
// Gates
//===----------------------------------------------------------------------===//

bool GNet::hasCombFlow(GateId gid, const SignalList &inputs) const {
  if (inputs.empty() || Gate::get(gid)->isTrigger()) {
    return false;
  }

  std::unordered_set<GateId> targets;
  targets.reserve(inputs.size());

  for (const auto input : inputs) {
    if (gid == input.node()) {
      return true;
    }
    targets.insert(input.node());
  }

  // BFS for checking if some of the inputs are reachable.
  std::unordered_set<GateId> reached;
  reached.reserve(_gates.size());

  std::queue<GateId> queue;
  queue.push(gid);

  while (!queue.empty()) {
    const auto *gate = Gate::get(queue.front());
    queue.pop();

    for (auto link : gate->links()) {
      const auto target = link.target;
      const auto status = reached.insert(target);

      if (targets.find(target) != targets.end()) {
        return true;
      }

      if (status.second && !Gate::get(target)->isTrigger()) {
        queue.push(target);
      }
    }
  }

  return false;
}

GNet::GateId GNet::addGate(GateSymbol func, const SignalList &inputs) {
  // Outputs are not shared (even equivalent ones).
  if (func == GateSymbol::OUT) {
    return addGate(new Gate(func, inputs));
  }

  // Search for the same gate.
  auto *gate = Gate::get(_id, func, inputs);

  // Do not create a gate if there exists the same one.
  if (gate != nullptr) {
    return addGateIfNew(gate);
  }

  // Try to decompose the node.
  if (func.isDecomposable()) {
    // For example: NAND(x, y) = NOT-AND(x, y).
    auto modifier = func.modifier(); // NOT
    auto baseFunc = func.function(); // AND

    // Search for the base node.
    auto *base = Gate::get(_id, baseFunc, inputs);
    if (base != nullptr) {
      addGateIfNew(base);
      gate = Gate::get(_id, modifier, {base->id()});

      if (gate != nullptr) {
        return addGateIfNew(gate);
      }

      return addGate(new Gate(modifier, {base->id()}));
    }
  }

  return addGate(new Gate(func, inputs));
}

GNet::GateId GNet::addGate(Gate *gate, SubnetId sid) {
  const auto gid = gate->id();
  assert(_flags.find(gid) == _flags.end());

  unsigned gindex = _gates.size();
  _gates.push_back(gate);

  GateFlags flags{0, sid, gindex};
  _flags.insert({gid, flags});

  onAddGate(gate, true, false);

  // Do some integrity checks.
  assert(!(_sourceLinks.empty() &&
           _targetLinks.empty() &&
           _constants.empty() &&
           _triggers.empty()));

  // Structural hashing.
  Gate::add(_id, gate);

  return gate->id();
}

void GNet::setGate(GateId gid, GateSymbol func, const SignalList &inputs) {
  // ASSERT: All inputs belong to the net (no need to modify the upper nets).
  // ASSERT: Adding the given inputs does not lead to combinational cycles.
  assert(contains(gid));

  auto *gate = Gate::get(gid);

  std::vector<GNet*> subnets;
  for (auto *subnet = this; subnet != nullptr;) {
    subnets.push_back(subnet);

    auto sid = subnet->getSubnetId(gid);
    subnet = (sid != INV_SUBNET) ? subnet->_subnets[sid] : nullptr;
  }

  std::for_each(subnets.begin(), subnets.end(), [=](GNet *subnet) {
    subnet->onRemoveGate(gate, true, true);
  });

  gate->setFunc(func);
  gate->setInputs(inputs);
  assert(gate->invariant());

  std::for_each(subnets.rbegin(), subnets.rend(), [=](GNet *subnet) {
    subnet->onAddGate(gate, true, true);
  });

  // Do some integrity checks.
  assert(!(_sourceLinks.empty() &&
           _targetLinks.empty() &&
           _constants.empty() &&
           _triggers.empty()));
}

void GNet::removeGate(GateId gid) {
  auto i = _flags.find(gid);
  assert(i != _flags.end());

  if (_gates.size() == 1) {
    clear();
    return;
  }

  const auto &flags = i->second;

  // If the net is hierarchical, do it recursively.
  if (flags.subnet != INV_SUBNET) {
    auto *subnet = _subnets[flags.subnet];

    subnet->removeGate(gid);
    _nGatesInSubnets--;

    if (subnet->isEmpty()) {
      _emptySubnets.insert(flags.subnet);
    }
  }

  auto *gate = Gate::get(gid);
  auto *last = _gates.back();

  _gates[flags.gindex] = last;
  _gates.erase(std::end(_gates) - 1, std::end(_gates));
  getFlags(last->id()).gindex = flags.gindex;

  onRemoveGate(gate, true, false);
  _flags.erase(i);

  // Do some integrity checks.
  assert((_sourceLinks.empty() &&
          _targetLinks.empty() &&
          _constants.empty() &&
          _triggers.empty()) == _gates.empty());
}

void GNet::onAddGate(Gate *gate, bool updateBoundary, bool withLinks) {
  const auto gid = gate->id();

  if (updateBoundary) {
    updateBoundaryLinksOnAdd(gate, withLinks);
  }

  if (gate->isValue()) {
    _constants.insert(gid);
  } else if (gate->isTrigger()) {
    _triggers.insert(gid);
  }

  _nConnects += gate->arity();
  _isSorted = (_gates.size() <= 1);
}

void GNet::onRemoveGate(Gate *gate, bool updateBoundary, bool withLinks) {
  const auto gid = gate->id();

  if (updateBoundary) {
    updateBoundaryLinksOnRemove(gate, withLinks);
  }

  if (gate->isValue()) {
    _constants.erase(gid);
  } else if (gate->isTrigger()) {
    _triggers.erase(gid);
  }

  _nConnects -= gate->arity();
  _isSorted = (_gates.size() <= 1);
}

void GNet::updateBoundaryLinksOnAdd(Gate *gate, bool withLinks) {
  const auto gid = gate->id();

  // Remove the links that became internal from the boundary.
  if (!withLinks) {
    // Update the source boundary.
    for (auto link : gate->links()) {
      _sourceLinks.erase(link);
    }
    // Update the target boundary.
    for (size_t i = 0; i < gate->arity(); i++) {
      const auto source = gate->input(i).node();
      _targetLinks.erase(Link(source, gid, i));
    }
  }

  // Add the links to the source boundary.
  if (gate->isSource()) {
    // If the gate is a pure source, add the source link.
    _sourceLinks.insert(Link(gid));
  } else {
    // Add the newly appeared boundary source links.
    for (size_t i = 0; i < gate->arity(); i++) {
      const auto source = gate->input(i).node();
      if (!contains(source)) {
        _sourceLinks.insert(Link(source, gid, i));
      }
    }
  }

  // Add the links to the target boundary.
  if (gate->isTarget()) {
    // If the gate is a pure target, add the target link.
    _targetLinks.insert(Link(gid));
  } else {
    // Add the newly appeared boundary target links.
    for (auto link : gate->links()) {
      const auto target = link.target;
      if (!contains(target)) {
        _targetLinks.insert(link);
      }
    }
  }
}

void GNet::updateBoundaryLinksOnRemove(Gate *gate, bool withLinks) {
  const auto gid = gate->id();

  // Remove the links from the source boundary.
  if (gate->isSource()) {
    // If the gate is a pure source, remove the source link.
    _sourceLinks.erase(Link(gid));
  } else {
    // Remove the previously existing boundary source links.
    for (size_t i = 0; i < gate->arity(); i++) {
      const auto source = gate->input(i).node();
      _sourceLinks.erase(Link(source, gid, i));
    }
  }

  // Remove the links from the target boundary.
  if (gate->isTarget()) {
    // If the gate is a pure target, remove the target link.
    _targetLinks.erase(Link(gid));
  } else {
    // Remove the previously existing boundary target links.
    for (auto link : gate->links()) {
      _targetLinks.erase(link);
    }
  }

  // Add the links that became boundary.
  if (!withLinks) {
    // Update the source boundary.
    for (auto link : gate->links()) {
      const auto target = link.target;
      if (contains(target)) {
        _sourceLinks.insert(link);
      }
    }
    // Update the target boundary.
    for (size_t i = 0; i < gate->arity(); i++) {
      const auto source = gate->input(i).node();
      if (contains(source)) {
        _targetLinks.insert(Link(source, gid, i));
      }
    }
  }
}

//===----------------------------------------------------------------------===//
// Subnets
//===----------------------------------------------------------------------===//

GNet::SubnetId GNet::newSubnet() {
  return addSubnet(new GNet(_level + 1));
}

void GNet::addNet(const GNet &net) {
  // ASSERT: this net and the given one are disjoint.
  const auto nG = _gates.size();
  const auto nS = _subnets.size();

  _gates.insert(std::end(_gates),
      std::begin(net._gates), std::end(net._gates));
  _subnets.insert(std::end(_subnets),
      std::begin(net._subnets), std::end(net._subnets));
  _emptySubnets.insert(
      std::begin(net._emptySubnets), std::end(net._emptySubnets));
  _constants.insert(
      std::begin(net._constants), std::end(net._constants));
  _triggers.insert(
      std::begin(net._triggers), std::end(net._triggers));

  _nConnects += net._nConnects;
  _nGatesInSubnets += net._nGatesInSubnets;

  discard_if(_sourceLinks, [this](Link link) {
    return !checkSourceLink(link);
  });
  for (auto link : net._sourceLinks) {
    if (checkSourceLink(link)) {
      _sourceLinks.insert(link);
    }
  }

  discard_if(_targetLinks, [this](Link link) {
    return !checkTargetLink(link);
  });
  for (auto link : net._targetLinks) {
    if (checkTargetLink(link)) {
      _targetLinks.insert(link);
    }
  }

  for (const auto &[gid, flags] : net._flags) {
    auto newFlags = flags;
    newFlags.subnet += nS;
    newFlags.gindex += nG;

    _flags.insert({gid, newFlags});
  }
}

GNet::SubnetId GNet::addSubnet(GNet *subnet) {
  // ASSERT: this net and the subnet are disjoint.
  assert(subnet);

  SubnetId sid = _subnets.size();
  _subnets.push_back(subnet);

  if (subnet->isEmpty()) {
    _emptySubnets.insert(sid);
  } else {
    for (auto *gate : subnet->gates()) {
      addGate(gate, sid);
    }
  }

  return sid;
}

void GNet::moveGate(GateId gid, SubnetId dst) {
  assert(dst == INV_SUBNET || dst < _subnets.size());

  const auto i = _flags.find(gid);
  assert(i != _flags.end());

  const auto src = i->second.subnet;
  assert(src == INV_SUBNET || src < _subnets.size());

  if (src == dst)
    return;

  if (src != INV_SUBNET) {
    auto *subnet = _subnets[src];

    subnet->removeGate(gid);
    _nGatesInSubnets--;

    if (subnet->isEmpty()) {
      _emptySubnets.insert(src);
    }
  }

  if (dst != INV_SUBNET) {
    auto *subnet = _subnets[dst];

    subnet->addGate(Gate::get(gid));
    _nGatesInSubnets++;
  }

  i->second.subnet = dst;
}

GNet::SubnetId GNet::mergeSubnets(SubnetId lhs, SubnetId rhs) {
  assert(lhs != INV_SUBNET && lhs < _subnets.size());
  assert(rhs != INV_SUBNET && rhs < _subnets.size());

  if (lhs == rhs)
    return lhs;

  auto *lhsSubnet = _subnets[lhs];
  auto *rhsSubnet = _subnets[rhs];

  // Do merging: lhs = merge(lhs, rhs).
  for (const auto *gate : rhsSubnet->gates()) {
    getFlags(gate->id()).subnet = lhs;
  }

  lhsSubnet->addNet(*rhsSubnet);
  rhsSubnet->clear();

  // Empty subnets are removed by request.
  _emptySubnets.insert(rhs);

  return lhs;
}

GNet::SubnetId GNet::groupOrphans() {
  assert(!isFlat() && hasOrphans());

  const auto sid = newSubnet();
  auto *subnet = _subnets[sid];

  // This is a slow operation.
  for (auto *gate : _gates) {
    GateFlags &flags = getFlags(gate->id());

    if (flags.subnet == INV_SUBNET) {
      flags.subnet = sid;
      subnet->addGate(gate);

      _nGatesInSubnets++;
      if (_nGatesInSubnets == _gates.size())
        break;
    }
  }

  assert(_nGatesInSubnets == _gates.size());
  return sid;
}

void GNet::flatten() {
  for (auto &[gid, flags] : _flags) {
    flags.subnet = INV_SUBNET;
  }

  _subnets.clear();
  _emptySubnets.clear();
  _nGatesInSubnets = 0;
}

void GNet::removeEmptySubnets() {
  if (_emptySubnets.empty())
    return;

  // ASSUME: indices of the empty subnets are sorted.
  unsigned nRemoved = 0;

  auto removeIt = _emptySubnets.begin();
  auto removeId = *removeIt;
  for (auto i = removeId; i < _subnets.size(); i++) {
    if (i == removeId) {
      nRemoved++;
      removeIt++;
      removeId = (removeIt == _emptySubnets.end()) ? INV_SUBNET : *removeIt;
      continue;
    }

    // Modifies the subnet index.
    auto *subnet = _subnets[i];
    for (const auto *gate : subnet->gates()) {
      getFlags(gate->id()).subnet -= nRemoved;
    }

    // Shift the subnets.
    _subnets[i - nRemoved] = subnet;
  }

  // Remove the empty subnets.
  _subnets.erase(std::end(_subnets) - nRemoved, std::end(_subnets));
  _emptySubnets.clear();
}

void GNet::clear() {
  _gates.clear();
  _flags.clear();
  _sourceLinks.clear();
  _targetLinks.clear();
  _constants.clear();
  _triggers.clear();
  _subnets.clear();
  _emptySubnets.clear();

  _nConnects = _nGatesInSubnets = 0;
  _isSorted = true;
}

//===----------------------------------------------------------------------===//
// Transforms
//===----------------------------------------------------------------------===//

/// Subnet-level graph of the net.
struct Subgraph final {
  using V = GNet*;
  using E = V;

  Subgraph(GNet &net): nV(net.nSubnets()), nE(0) {
    sources.reserve(nV);

    for (auto *subnet : net.subnets()) {
      // Collect sources: subnets w/ global sources or trigger-based sources.
      for (auto sourceLink : subnet->sourceLinks()) {
        auto *gate = Gate::get(sourceLink.source);
        if (net.hasSourceLink(sourceLink) || gate->isTrigger()) {
          sources.push_back(subnet);
          break;
        }
      }

      // Collect edges.
      auto &outEdges = edges[subnet];
      for (auto targetLink : subnet->targetLinks()) {
        auto *gate = Gate::get(targetLink.source);
        if (gate->isTrigger()) continue;

        for (auto link : gate->links()) {
          auto  sid = net.getSubnetId(link.target);
          auto *end = const_cast<GNet*>(net.subnet(sid));

          if (end != subnet) {
            outEdges.insert(end);
          }
        }
      }

      nE += outEdges.size();
    }
  }

  size_t nNodes() const { return nV; }
  size_t nEdges() const { return nE; }

  bool hasNode(V v) const { return true; }
  bool hasEdge(E e) const { return true; }

  const std::vector<V> &getSources() const {
    return sources;
  }

  const std::unordered_set<E> &getOutEdges(V v) const {
    return edges.find(v)->second;
  }

  V leadsTo(E e) const {
    return e;
  }

  size_t nV;
  size_t nE;

  std::vector<V> sources;
  std::unordered_map<V, std::unordered_set<E>> edges;
};

void GNet::sortTopologically() {
  assert(isWellFormed());

  if (_isSorted)
    return;

  _isSorted = true;

  // If the net is flat, sort the gates and update the indices.
  if (isFlat()) {
    auto gates = topologicalSort<GNet>(*this);

    for (size_t i = 0; i < gates.size(); i++) {
      auto gid = gates[i];

      _gates[i] = Gate::get(gid);
      getFlags(gid).gindex = i;
    }

    return;
  }

  // If the net is hierarchical, sort the subnets.
  Subgraph subgraph(*this);

  using G = Subgraph;
  using E = G::E;
  auto subnets = topologicalSort<G, std::unordered_set<E>>(subgraph);

  // Sort each subnet.
  for (auto *subnet : subnets) {
    subnet->sortTopologically();
  }

  // Sort the gates and update the indices.
  size_t offset = 0;
  for (auto *subnet : subnets) {
    for (size_t i = 0; i < subnet->nGates(); i++) {
      auto gid = subnet->gate(i)->id();
      auto j = offset + i;

      _gates[j] = Gate::get(gid);
      getFlags(gid).gindex = j;
    }

    offset += subnet->nGates();
  }
}

//===--------------------------------------------------------------------===//
// Cloning
//===--------------------------------------------------------------------===//

/// Clones the net.
GNet *GNet::clone() {
  if (_gates.empty()) {
    return new GNet(_level);
  }

  std::unordered_map<Gate::Id, Gate::Id> oldToNewId = {};
  return clone(oldToNewId);
}

GNet *GNet::clone(std::unordered_map<Gate::Id, Gate::Id> &oldToNewId) {
  auto *resultNet = new GNet(_level);
  assert(oldToNewId.empty());

  for (auto *gate : _gates) {
    oldToNewId[gate->id()] = resultNet->newGate();
  }

  for (auto *gate : _gates) {

    SignalList newSignals;
    newSignals.reserve(gate->_inputs.capacity());

    for (auto signal : gate->inputs()) {
      newSignals.emplace_back(signal.event(), oldToNewId[signal.node()]);
    }

    auto newGateId = oldToNewId[gate->id()];
    resultNet->setGate(newGateId, gate->func(), newSignals);
  }

  if (!_subnets.empty()) {
    for (auto *subnet : _subnets) {
      resultNet->addSubnet(subnet->clone(oldToNewId));
    }
  }

  resultNet->sortTopologically();
  return resultNet;
}

//===----------------------------------------------------------------------===//
// Output
//===----------------------------------------------------------------------===//

std::ostream& operator <<(std::ostream &out, const GNet &net) {
  for (const auto *gate: net.gates()) {
    out << *gate << std::endl;
  }
  return out;
}

} // namespace eda::gate::model
