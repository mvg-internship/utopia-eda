//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/premapper/aigmapper.h"
#include "util/singleton.h"

namespace eda::gate::premapper {

/**
 * \brief Implements an netlist-to-XAG pre-mapper.
 * \author <a href="mailto:nsromanov_1@edu.hse.ru">Nikita Romanov</a>
 */
class XagMapper : public AigMapper, public util::Singleton<XagMapper> {
  friend class util::Singleton<XagMapper>;

protected:
  Gate::Id mapXor(const Gate::SignalList &newInputs,
                  bool sign, GNet &newNet) const override;
};

} // namespace eda::gate::premapper
