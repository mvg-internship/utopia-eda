//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/premapper/migmapper.h"
#include "util/singleton.h"

namespace eda::gate::premapper {

/**
 * \brief Implements an netlist-to-XMG pre-mapper.
 * \author <a href="mailto:mdvershkov@edu.hse.ru">Maksim Vershkov</a>
 */
class XmgMapper : public MigMapper, public util::Singleton<XmgMapper> {
  friend class util::Singleton<XmgMapper>;

protected:
  Gate::Id mapXor(const Gate::SignalList &newInputs,
                  bool sign, GNet &newNet) const override;
};

} // namespace eda::gate::premapper
