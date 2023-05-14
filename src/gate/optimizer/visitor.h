//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/cut_storage.h"

namespace eda::gate::optimizer {
  enum VisitorFlags {
    SUCCESS,
    FINISH_ALL,
    FINISH_THIS
  };

/**
 * \brief Interface to handle node and its cuts.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  class Visitor {
  public:

    using GNet = eda::gate::model::GNet;
    using GateID = GNet::GateId;
    using Cut = CutStorage::Cut;

    virtual VisitorFlags onNodeBegin(const GateID &) = 0;

    virtual VisitorFlags onNodeEnd(const GateID &) = 0;

    virtual VisitorFlags onCut(const Cut &) = 0;
  };
} // namespace eda::gate::optimizer
