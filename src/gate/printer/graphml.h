//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"

#include <ostream>

namespace eda::printer::graphMl {

/**
* \brief Converts GNet to GraphMl representation.
* \author <a href="mailto:alex.sh2002@mail.ru">Alexsey Shtokman</a>
*/
class toGraphMl {
  using GNet = eda::gate::model::GNet;
  using Gate = eda::gate::model::Gate;
  using Link = Gate::Link;
  public:
    static void printer(std::ostream &output, const GNet &model);
  private:
    static std::string linkDescription(const Link &link);
};

} //namespace eda::printer::graphMl