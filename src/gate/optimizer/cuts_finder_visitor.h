//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/optimizer/cut_storage.h"
#include "gate/optimizer/visitor.h"

namespace eda::gate::optimizer {
/**
 * \brief Realization of interface Visitor.
 * \ Finds cuts in given net.
 * \author <a href="mailto:dreamer_1977@ispras.ru">Liza Shcherbakova</a>
 */
  class CutsFindVisitor : public Visitor {

    int cutSize;
    CutStorage *cutStorage;

  public:

    CutsFindVisitor(int cutSize, CutStorage *cutStorage);

    VisitorFlags onNodeBegin(const GateID &) override;

    VisitorFlags onNodeEnd(const GateID &) override;

    VisitorFlags onCut(const Cut &) override;
  };
} // namespace eda::gate::optimizer
