//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/debugger/base_checker.h"
#include "gate/debugger/context.h"
#include "gate/debugger/encoder.h"
#include "gate/model/gnet.h"
#include "gate/premapper/premapper.h"
#include "util/singleton.h"

#include <memory>
#include <unordered_map>

namespace eda::gate::debugger {

/**
 * \brief Implements a logic equivalence checker (LEC).
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */

class Checker : public BaseChecker, public util::Singleton<Checker> {
friend class util::Singleton<Checker>;

public:
  using Gate = eda::gate::model::Gate;
  using GNet = eda::gate::model::GNet;

  using GateBinding = std::unordered_map<Gate::Link, Gate::Link>;
  using GateConnect = Context::GateConnect;
  using GateIdMap = eda::gate::premapper::PreMapper::GateIdMap;
  using SubnetBinding = std::unordered_map<GNet::SubnetId, GNet::SubnetId>;

  /// Represents LEC hints.
  struct Hints final {
    // Known correspondence between input/output ports.
    bool isKnownIoPortBinding() const {
      return sourceBinding != nullptr;
    }

    std::shared_ptr<GateBinding> sourceBinding;
    std::shared_ptr<GateBinding> targetBinding;

    // Known correspondence between triggers.
    bool isKnownTriggerBinding() const {
      return triggerBinding != nullptr;
    }

    std::shared_ptr<GateBinding> triggerBinding;

    // Known state encoding/decoding logic.
    bool isKnownStateEncoding() const {
      return encoder != nullptr;
    }

    std::shared_ptr<GNet> encoder;
    std::shared_ptr<GNet> decoder;
    std::shared_ptr<GateBinding> lhsTriEncIn;
    std::shared_ptr<GateBinding> lhsTriDecOut;
    std::shared_ptr<GateBinding> rhsTriEncOut;
    std::shared_ptr<GateBinding> rhsTriDecIn;

    // Known correspondence between subnets.
    bool isKnownSubnetBinding() const {
      return subnetBinding != nullptr;
    }

    std::shared_ptr<SubnetBinding> subnetBinding;

    // Correspondence between some inner gates, including subnet boundaries.
    bool isKnownInnerBinding() const {
      return innerBinding != nullptr;
    }

    std::shared_ptr<GateBinding> innerBinding;
  };

  /// Checks logic equivalence of two nets.
  bool areEqual(const GNet &lhs,
                const GNet &rhs,
                const Hints &hints) const;
  
  bool areEqual(GNet &lhs,
                GNet &rhs,
                GateIdMap &gmap) override;

private:
  /// Checks logic equivalence of two hierarchical nets.
  bool areEqualHier(const GNet &lhs,
                    const GNet &rhs,
                    const Hints &hints) const;

  /// Checks logic equivalence of two flat combinational nets.
  bool areEqualComb(const GNet &lhs,
                    const GNet &rhs,
	            const GateBinding &ibind,
	            const GateBinding &obind) const;

  /// Checks logic equivalence of two flat sequential nets
  /// with one-to-one correspondence of triggers.
  bool areEqualSeq(const GNet &lhs,
                   const GNet &rhs,
                   const GateBinding &ibind,
                   const GateBinding &obind,
                   const GateBinding &tbind) const;

  /// Checks logic equivalence of two flat sequential nets
  /// with given correspondence of state encodings.
  bool areEqualSeq(const GNet &lhs,
                   const GNet &rhs,
                   const GNet &enc,
                   const GNet &dec,
                   const GateBinding &ibind,
                   const GateBinding &obind,
                   const GateBinding &lhsTriEncIn,
                   const GateBinding &lhsTriDecOut,
                   const GateBinding &rhsTriEncOut,
                   const GateBinding &rhsTriDecIn) const;

  /// Simulation-based LEC of two small combinational nets by
  /// applying all possible inputs and checking the outputs.
  bool areEqualCombSim(const GNet &lhs,
                       const GNet &rhs,
                       const GateBinding &ibind,
                       const GateBinding &obind) const;

  /// SAT-based LEC of two flat combinational nets.
  bool areEqualCombSat(const std::vector<const GNet*> &nets,
                       const GateConnect *connectTo,
	               const GateBinding &ibind,
	               const GateBinding &obind) const;

  /// Handles an error (prints the diagnostics, etc.).
  void error(Context &context,
	     const GateBinding &ibind,
	     const GateBinding &obind) const;
};
} // namespace eda::gate::debugger
