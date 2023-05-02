//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/premapper/premapper.h"
#include "util/singleton.h"

namespace eda::gate::premapper {

/**
 * \brief Implements a netlist-to-MIG pre-mapper.
 * \author <a href="mailto:mdvershkov@edu.hse.ru">Maksim Vershkov</a>
 */
class MigMapper : public PreMapper, public util::Singleton<MigMapper> {
  friend class util::Singleton<MigMapper>;

protected:
  Gate::Id mapGate(const Gate &oldGate,
                   const GateIdMap &oldToNewGates,
                   GNet &newNet) const override;

  Gate::Id mapIn(GNet &newNet) const override;
  Gate::Id mapOut(const Gate::SignalList &newInputs,
                  const size_t n0, const size_t n1,
                  GNet &newNet) const override;

  Gate::Id mapVal(const bool value, GNet &newNet) const override;

  Gate::Id mapNop(const Gate::SignalList &newInputs,
                  const bool sign, GNet &newNet) const override;
  Gate::Id mapNop(const Gate::SignalList &newInputs,
                  const size_t n0, const size_t n1,
                  const bool sign, GNet &newNet) const override;

  Gate::Id mapAnd(const Gate::SignalList &newInputs,
                  const bool sign, GNet &newNet) const override;
  Gate::Id mapAnd(const Gate::SignalList &newInputs,
                  const size_t n0, const size_t n1,
                  const bool sign, GNet &newNet) const override;

  Gate::Id mapOr(const Gate::SignalList &newInputs,
                 const bool sign, GNet &newNet) const override;
  Gate::Id mapOr(const Gate::SignalList &newInputs,
                 const size_t n0, const size_t n1,
                 const bool sign, GNet &newNet) const override;

  Gate::Id mapXor(const Gate::SignalList &newInputs,
                  bool sign, GNet &newNet) const override;
  Gate::Id mapXor(const Gate::SignalList &newInputs,
                  const size_t n0, const size_t n1,
                  const bool sign, GNet &newNet) const override;

  Gate::Id mapMaj(const Gate::SignalList &newInputs,
                  const size_t n0, const size_t n1, GNet &newNet) const;

};

} // namespace eda::gate::premapper