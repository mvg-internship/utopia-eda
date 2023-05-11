//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gsymbol.h"

#include <cassert>
#include <iostream>

namespace eda::gate::model {

GateSymbol::GateDescriptor GateSymbol::_desc[N_GATE_SYMBOLS] = {
  /* IN    */ { "in",     0, 0, 0, 0, 0, XXX, XXX  },
  /* OUT   */ { "out",    0, 0, 0, 0, 0, XXX, XXX  },
  /* ZERO  */ { "0",      1, 0, 0, 0, 0, XXX, XXX  },
  /* ONE   */ { "1",      1, 0, 0, 0, 0, XXX, XXX  },
  /* NOP   */ { "buf",    0, 1, 0, 0, 0, XXX, XXX  },
  /* NOT   */ { "not",    0, 0, 0, 0, 0, XXX, XXX  },
  /* AND   */ { "and",    0, 0, 1, 1, 1, NOT, NAND },
  /* OR    */ { "or",     0, 0, 1, 1, 1, NOT, NOR  },
  /* XOR   */ { "xor",    0, 0, 1, 1, 1, NOT, XNOR },
  /* NAND  */ { "nand",   0, 0, 1, 0, 1, NOT, AND  },
  /* NOR   */ { "nor",    0, 0, 1, 0, 1, NOT, OR   },
  /* XNOR  */ { "xnor",   0, 0, 1, 1, 1, NOT, XOR  },
  /* MAJ   */ { "maj",    0, 0, 1, 0, 0, XXX, XXX  },
  /* LATCH */ { "latch",  0, 0, 0, 0, 0, XXX, XXX  },
  /* DFF   */ { "dff",    0, 0, 0, 0, 0, XXX, XXX  },
  /* DFFrs */ { "dff_rs", 0, 0, 0, 0, 0, XXX, XXX  }
};

uint16_t GateSymbol::_next = GateSymbol::XXX;

GateSymbol GateSymbol::create(const std::string &name) {
  assert(_next < N_GATE_SYMBOLS);
  _desc[_next] = { name, 0, 0, 0, 0, 0, XXX, XXX };
  return static_cast<Value>(_next++);
}

std::ostream& operator <<(std::ostream &out, GateSymbol gate) {
  return out << gate.name();
}

} // namespace eda::gate::model
