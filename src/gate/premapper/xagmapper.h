//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/premapper/premapper.h"
#include "util/singleton.h"

namespace eda::gate::premapper {

/**
 * \brief Implements an netlist-to-XAG pre-mapper.
 * \author <a href="mailto:nsromanov_1@edu.hse.ru">Nikita Romanov</a>
 */
class XagMapper final : public PreMapper, public util::Singleton<XagMapper> {
  friend class util::Singleton<XagMapper>;

protected:
  Gate::Id mapGate(const Gate &oldGate,
                   const GateIdMap &oldToNewGates,
                   GNet &newNet) const override;

private:
  Gate::Id mapVal(bool value, GNet &newNet) const;

  Gate::Id mapNop(const Gate::SignalList &newInputs,
                  bool sign, GNet &newNet) const;
  Gate::Id mapNop(const Gate::SignalList &newInputs,
                  size_t n0, size_t n1, bool sign, GNet &newNet) const;

  Gate::Id mapAnd(const Gate::SignalList &newInputs,
                  bool sign, GNet &newNet) const;
  Gate::Id mapAnd(const Gate::SignalList &newInputs,
                  size_t n0, size_t n1, bool sign, GNet &newNet) const;

  Gate::Id mapOr (const Gate::SignalList &newInputs,
                  bool sign, GNet &newNet) const;
  Gate::Id mapOr (const Gate::SignalList &newInputs,
                  size_t n0, size_t n1, bool sign, GNet &newNet) const;

  Gate::Id mapXor(const Gate::SignalList &newInputs,
                  bool sign, GNet &newNet) const;
  Gate::Id mapXor(const Gate::SignalList &newInputs,
                  size_t n0, size_t n1, bool sign, GNet &newNet) const;

};

} // namespace eda::gate::premapper

