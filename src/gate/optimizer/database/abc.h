//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

namespace eda::gate::optimizer {

class RWDatabase;

void initializeAbcRwDatabase(RWDatabase &database);

} // namespace eda::gate::optimizer
