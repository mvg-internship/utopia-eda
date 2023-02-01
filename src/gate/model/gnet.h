//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gate.h"

#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace eda::gate::premapper {
  class PreMapper;
} // namespace eda::gate::premapper

namespace eda::rtl::compiler {
  class Compiler;
} // namespace eda::rtl::compiler

namespace eda::gate::model {

/**
 * \brief Represents a (hierarchical) gate-level net.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class GNet final {
  friend class eda::gate::premapper::PreMapper;
  friend class eda::rtl::compiler::Compiler;

public:
  //===--------------------------------------------------------------------===//
  // Types
  //===--------------------------------------------------------------------===//

  using List        = std::vector<GNet*>;
  using GateId      = Gate::Id;
  using GateIdList  = std::vector<GateId>;
  using GateIdSet   = std::unordered_set<GateId>;
  using SubnetId    = unsigned;
  using SubnetIdSet = std::set<SubnetId>;
  using Link        = Gate::Link;
  using LinkList    = Gate::LinkList;
  using LinkSet     = std::unordered_set<Link>;
  using Signal      = Gate::Signal;
  using SignalList  = Gate::SignalList;
  using Value       = std::vector<bool>;
  using In          = std::vector<GateIdList>;
  using Out         = GateIdList;

  #pragma pack(push,1)
  struct GateFlags {
    /// Gate flags (reserved).
    unsigned gflags : 12;
    /// Local index of the gate's subnet.
    unsigned subnet : 20;
    /// Local index of the gate.
    unsigned gindex : 32;
  };
  #pragma pack(pop)

  //===--------------------------------------------------------------------===//
  // Constants
  //===--------------------------------------------------------------------===//

  /// Invalid subnet index.
  static constexpr SubnetId INV_SUBNET = (1u << 20) - 1;
  /// Maximum subnet index (max. 2^20 - 1 subnets).
  static constexpr SubnetId MAX_SUBNET = INV_SUBNET - 1;

  //===--------------------------------------------------------------------===//
  // Constructors/Destructors
  //===--------------------------------------------------------------------===//

  /// Constructs an empty net (level=0 stands for the top level).
  explicit GNet(unsigned level = 0);
  /// Destructs the net.
  ~GNet() = default;

  //===--------------------------------------------------------------------===//
  // Properties 
  //===--------------------------------------------------------------------===//

  /// Checks whether the net is top-level.
  bool isTop() const {
    return _level == 0;
  }

  /// Checks whether the net is flat.
  bool isFlat() const {
    return _subnets.empty();
  }

  /// Checks whether the net is empty.
  bool isEmpty() const {
    return _gates.empty();
  }

  /// Checks whether the net has orphans.
  bool hasOrphans() const {
    return _nGatesInSubnets < _gates.size();
  }

  /// Checks whether the net has empty subnets.
  bool hasEmptySubnets() const {
    return !_emptySubnets.empty();
  }

  /// Checks whether the net is well-formed.
  bool isWellFormed() const {
    return isFlat() || (!hasOrphans() && !hasEmptySubnets());
  }

  /// Checks whether the net is combinational.
  bool isComb() const {
    return _triggers.empty();
  }

  /// Checks whether the net is topologically sorted.
  bool isSorted() const {
    return _isSorted;
  }

  //===--------------------------------------------------------------------===//
  // Statistics 
  //===--------------------------------------------------------------------===//

  unsigned id() const {
    return _id;
  }

  /// Returns the net level.
  unsigned getLevel() const {
    return _level;
  }

  /// Returns the number of gates.
  size_t nGates() const {
    return _gates.size();
  }

  /// Returns the number of input links.
  size_t nSourceLinks() const {
    return _sourceLinks.size();
  }

  /// Returns the number of output links.
  size_t nTargetLinks() const {
    return _targetLinks.size();
  }

  /// Returns the number of constants.
  size_t nConstants() const {
    return _constants.size();
  }

  /// Returns the number of triggers.
  size_t nTriggers() const {
    return _triggers.size();
  }

  /// Returns the number of connections.
  size_t nConnects() const {
    return _nConnects;
  }

  /// Returns the number of subnets.
  size_t nSubnets() const {
    return _subnets.size();
  }

  //===--------------------------------------------------------------------===//
  // Gates 
  //===--------------------------------------------------------------------===//

  /// Returns the collection of gates.
  const Gate::List &gates() const {
    return _gates;
  }

  /// Returns the collection input links.
  const LinkSet &sourceLinks() const {
    return _sourceLinks;
  }

  /// Returns the collection of output links.
  const LinkSet &targetLinks() const {
    return _targetLinks;
  }

  /// Returns the collection of constants.
  const GateIdSet &constants() const {
    return _constants;
  }

  /// Returns the collection of triggers.
  const GateIdSet &triggers() const {
    return _triggers;
  }

  /// Gets a gate by index.
  const Gate *gate(size_t index) const {
    return _gates[index];
  }

  /// Checks whether the net contains the gate.
  bool contains(GateId gid) const {
    return _flags.find(gid) != _flags.end();
  }

  /// Checks whether the net has the source link.
  bool hasSourceLink(const Link &link) const {
    return _sourceLinks.find(link) != _sourceLinks.end();
  }

  /// Checks whether the net has the target link.
  bool hasTargetLink(const Link &link) const {
    return _targetLinks.find(link) != _targetLinks.end();
  }

  /// Checks whether the net has the constant.
  bool hasConstant(GateId gid) const {
    return _constants.find(gid) != _constants.end();
  }

  /// Checks whether the net has the trigger.
  bool hasTrigger(GateId gid) const {
    return _triggers.find(gid) != _triggers.end();
  }

  /// Checks if any of the given inputs depends on the given gate.
  bool hasCombFlow(GateId gid, const SignalList &inputs) const;

  /// Adds a new (empty) gate and returns its identifier.
  GateId newGate() {
    return addGate(new Gate());
  }

  /// Adds a new gate and returns its identifier.
  GateId addGate(GateSymbol func, const SignalList &inputs);

  /// Modifies the existing gate.
  void setGate(GateId gid, GateSymbol func, const SignalList &inputs);

  /// Removes the gate from the net.
  void removeGate(GateId gid);

  //===--------------------------------------------------------------------===//
  // Subnets 
  //===--------------------------------------------------------------------===//

  /// Returns the collection of subnets.
  const List &subnets() const {
    return _subnets;
  }

  /// Gets a subnet by index.
  const GNet *subnet(size_t index) const {
    return _subnets[index];
  }

  /// Checks whether the gate is an orphan (does not belong to any subnet).
  bool isOrphan(GateId gid) const {
    return getFlags(gid).subnet == INV_SUBNET;
  }

  /// Returns the subnet the given gate belongs to.
  SubnetId getSubnetId(GateId gid) const {
    return getFlags(gid).subnet;
  }

  /// Adds a new (empty) subnet and returns its identifier.
  SubnetId newSubnet();

  /// Adds the content of the given net.
  void addNet(const GNet &net);

  /// Moves the gate to the given subnet.
  void moveGate(GateId gid, SubnetId dst);

  /// Merges the subnets and returns the identifier of the joint subnet.
  SubnetId mergeSubnets(SubnetId lhs, SubnetId rhs);

  /// Combines all orphan gates into a subnet.
  SubnetId groupOrphans();

  /// Flattens the net (removes the hierarchy).
  void flatten();

  /// Removes the empty subnets.
  void removeEmptySubnets();

  /// Clears the net.
  void clear();

  //===--------------------------------------------------------------------===//
  // Graph Interface
  //===--------------------------------------------------------------------===//

  using V = GateId;
  using E = Link;

  /// Returns the number of nodes.
  size_t nNodes() const {
    return nGates();
  }

  /// Returns the number of edges.
  size_t nEdges() const {
    return nConnects();
  }

  /// Checks whether the graph contains the node.
  bool hasNode(GateId gid) const {
    return contains(gid);
  }

  /// Checks whether the graph contains the edge.
  bool hasEdge(const Link &link) const {
    return !hasTrigger(link.target);
  }

  /// Returns the graph sources.
  GateIdSet getSources() const {
    GateIdSet sources;
    sources.reserve(_sourceLinks.size() + _triggers.size());

    for (auto link : _sourceLinks) {
      sources.insert(link.source);
    }
    for (auto constant : _constants) {
      sources.insert(constant);
    }
    for (auto trigger : _triggers) {
      sources.insert(trigger);
    }

    return sources;
  }

  /// Returns the outgoing edges of the node.
  const LinkList &getOutEdges(GateId gid) const {
    return Gate::get(gid)->links();
  }

  /// Returns the end of the edge.
  GateId leadsTo(const Link &link) const {
    return link.target;
  }

  //===--------------------------------------------------------------------===//
  // Transforms
  //===--------------------------------------------------------------------===//

  /// Sorts the gates in topological order.
  void sortTopologically();

  //===--------------------------------------------------------------------===//
  // Cloning 
  //===--------------------------------------------------------------------===//

  /// Clones the net
  GNet* clone();
  GNet* clone(std::unordered_map<Gate::Id, Gate::Id> &oldToNewId);

private:
  //===--------------------------------------------------------------------===//
  // Internal Methods
  //===--------------------------------------------------------------------===//

  /// Adds the gate to the net and sets the subnet index.
  /// The subnet is not modified.
  GateId addGate(Gate *gate, SubnetId sid = INV_SUBNET);

  /// Adds the gate to the net if the gate is a new one.
  GateId addGateIfNew(Gate *gate, SubnetId sid = INV_SUBNET) {
    if (contains(gate->id())) {
      return gate->id();
    }
    return addGate(gate, sid);
  }

  /// Adds the subnet to the net.
  SubnetId addSubnet(GNet *subnet);

  /// Returns a copy of the gate flags.
  GateFlags getFlags(GateId gid) const {
    return _flags.find(gid)->second;
  }

  /// Returns the reference to the gate flags.
  GateFlags &getFlags(GateId gid) {
    return _flags.find(gid)->second;
  }

  /// Checks whether the link is a source link.
  bool checkSourceLink(const Link &link) const {
    return link.isPort() || !contains(link.source);
  }

  /// Checks whether the link is a target link.
  bool checkTargetLink(const Link &link) const {
    return link.isPort() || !contains(link.target);
  }

  /// Updates the net state when adding a gate.
  void onAddGate(Gate *gate, bool withLinks);
  /// Updates the net state when removing a gate.
  void onRemoveGate(Gate *gate, bool withLinks);

  //===--------------------------------------------------------------------===//
  // Internal Fields
  //===--------------------------------------------------------------------===//

  /// Identifier.
  const unsigned _id;
  /// Level (0 = top level).
  const unsigned _level;

  /// Gates of the net.
  Gate::List _gates;

  /// Gate flags.
  std::unordered_map<GateId, GateFlags> _flags;

  /// Input links: {(external gate, internal gate, internal input)}.
  LinkSet _sourceLinks;
  /// Output links: {(internal gate, external gate, external input)}.
  LinkSet _targetLinks;

  // Constants.
  GateIdSet _constants;
  // Triggers.
  GateIdSet _triggers;

  /// Number of connections.
  size_t _nConnects;

  /// All subnets including the empty ones.
  List _subnets;
  /// Indices of the empty subnets.
  SubnetIdSet _emptySubnets;

  /// Number of gates that belong to subnets.
  size_t _nGatesInSubnets;

  /// Flag indicating that the net is topologically sorted.
  bool _isSorted;

  /// Counter for identifier initialization.
  static unsigned _counter;
};

/// Outputs the net.
std::ostream& operator <<(std::ostream &out, const GNet &net);

} // namespace eda::gate::model
