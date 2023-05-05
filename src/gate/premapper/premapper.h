//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

#include <memory>
#include <unordered_map>

namespace eda::gate::premapper {

/**
 * \brief Interface of a pre-mapper, which maps a netlist to the IR (e.g., AIG).
 * \author <a href="mailto:nsromanov_1@edu.hse.ru">Nikita Romanov</a>
 */
class PreMapper {
protected:
  using Gate = eda::gate::model::Gate;
  using GNet = eda::gate::model::GNet;

public:
  using GateIdMap = std::unordered_map<Gate::Id, Gate::Id>;

  /// Maps the given net to a new one and fills the gate correspondence map.
  std::shared_ptr<GNet> map(const GNet &net, GateIdMap &oldToNewGates) const;

  /// Maps the given net to a new one.
  std::shared_ptr<GNet> map(const GNet &net) const {
    GateIdMap oldToNewGates;
    oldToNewGates.reserve(net.nGates());

    return map(net, oldToNewGates);
  }

protected:
  virtual Gate::Id mapIn(GNet &newNet) const = 0;
  virtual Gate::Id mapOut(const Gate::SignalList &newInputs,
                          const size_t n0, const size_t n1,
                          GNet &newNet) const = 0;

  virtual Gate::Id mapVal(const bool value, GNet &newNet) const = 0;

  virtual Gate::Id mapNop(const Gate::SignalList &newInputs,
                          const bool sign, GNet &newNet) const = 0;
  virtual Gate::Id mapNop(const Gate::SignalList &newInputs,
                          const size_t n0, const size_t n1,
                          const bool sign, GNet &newNet) const = 0;

  virtual Gate::Id mapAnd(const Gate::SignalList &newInputs,
                          const bool sign, GNet &newNet) const = 0;
  virtual Gate::Id mapAnd(const Gate::SignalList &newInputs,
                          const size_t n0, const size_t n1,
                          const bool sign, GNet &newNet) const = 0;

  virtual Gate::Id mapOr(const Gate::SignalList &newInputs,
                         const bool sign, GNet &newNet) const = 0;
  virtual Gate::Id mapOr(const Gate::SignalList &newInputs,
                         const size_t n0, const size_t n1,
                         const bool sign, GNet &newNet) const = 0;

  virtual Gate::Id mapXor(const Gate::SignalList &newInputs,
                          bool sign, GNet &newNet) const = 0;
  virtual Gate::Id mapXor(const Gate::SignalList &newInputs,
                          const size_t n0, const size_t n1,
                          const bool sign, GNet &newNet) const = 0;

  PreMapper() {}
  virtual ~PreMapper() {}

  GNet *mapGates(const GNet &net, GateIdMap &oldToNewGates) const;

  /// Creates new gates representing the given one and adds them to the net.
  /// Returns the identifier of the new gate corresponding to the old one or
  /// Gate::INVALID if the operation fails.
  virtual Gate::Id mapGate(const Gate &oldGate,
                           const GateIdMap &oldToNewGates,
                           GNet &newNet) const;
};

/**
 * \brief Defines functional bases supported by pre-mappers.
 * \author <a href="mailto:smolov@ispras.ru">Sergey Smolov</a>
 */
enum PreBasis {
  /// And-Inverter Graph
  AIG,
  /// Maj-Inverter Graph
  MIG,
  /// Xor-And-Inverter Graph
  XAG,
  /// Xor-Maj-Inverter Graph
  XMG
};

/// Returns pre-mapper for the specified functional basis.
PreMapper &getPreMapper(PreBasis basis);

} // namespace eda::gate::premapper