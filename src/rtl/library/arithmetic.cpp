//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "rtl/library/arithmetic.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace eda::base::model;
using namespace eda::gate::model;
using namespace eda::rtl::model;

namespace eda::rtl::library {

bool ArithmeticLibrary::supports(FuncSymbol func) const {
  return true;
}

FLibrary::Out ArithmeticLibrary::synth(size_t outSize, 
                                       const Value &value, 
                                       GNet &net) {
  return supportLibrary.synth(outSize, value, net);
}

FLibrary::Out ArithmeticLibrary::synth(size_t outSize, 
                                       const Out &out, 
                                       GNet &net) {
  return supportLibrary.synth(outSize, out, net);
}

FLibrary::Out ArithmeticLibrary::synth(size_t outSize, 
                                       FuncSymbol func, 
                                       const In &in, 
                                       GNet &net) {
  switch (func) {
  case FuncSymbol::ADD:
    return synthAdd(outSize, in, net);
  case FuncSymbol::SUB:
    return synthSub(outSize, in, net);
  default:
    return supportLibrary.synth(outSize, func, in, net);
  }
}

FLibrary::Out ArithmeticLibrary::synth(const Out &out, 
                                       const In &in, 
                                       const SignalList &control, 
                                       GNet &net) {
  return supportLibrary.synth(out, in, control, net);
}

FLibrary::Out ArithmeticLibrary::alloc(size_t outSize, 
                                       GNet &net) {
  return supportLibrary.alloc(outSize, net);
}

FLibrary::Out ArithmeticLibrary::synthAdd(size_t outSize, 
                                          const In &in, 
                                          GNet &net) {
  return synthAdder(outSize, in, false, net);
}

FLibrary::Out ArithmeticLibrary::synthSub(size_t outSize, 
                                          const In &in, 
                                          GNet &net) {
  const auto &x = in[0];
  const auto &y = in[1];

  assert(outSize == y.size());

  Out temp(outSize);
  for (size_t i = 0; i < temp.size(); i++) {
    auto yWire = Signal::always(y[i]);
    temp[i] = net.addGate(GateSymbol::NOT, {yWire});
  }

  return synthAdder(outSize, {x, temp}, true, net);
}

//
//                              LADNER-FISHER ADDER
//
//                                                          Input
//                                                          carry    1
//  | 6 |   | 5 |   | 4 |   | 3 |   | 2 |   | 1 |   | 0 |   | -1| _________
//    |  _____|       |  _____|       |  _____|       |  _____|
//    | /     |       | /     |       | /     |       | /     |
//    |/      |       |/      |       |/      |       |/      |
//    X[6,5]  |       X[4,3]  |       X[2,1]  |       O[0,-1] |
//    |       |       |       |       |       |       |       |
//    |  _____________|       |       |  _____________|       |
//    | /     |       |       |       | /     |       |       |
//    |/      |       |       |       |/      |       |       |
//    X[6,3]  |       |       |       O[2,-1] |       |       |
//    |       |       |       |       |       |       |       |      2
//    |  _____________________________|       |       |       |
//    | /     |       | /     |       |       |       |       |
//    |/      |       |/      |       |       |       |       |
//    O[6,-1] |       O[4,-1] |       |       |       |       |
//    |       |       |       |       |       |       |       |
//    |       |  _____|       |  _____|       |  _____|       |
//    |       | /     |       | /     |       | /     |       |
//    |       |/      |       |/      |       |/      |       |
//    |       O[5,-1] |       O[3,-1] |       O[1,-1] |       |
//    |       |       |       |       |       |       |       |   _________
//  ( 7 )   ( 6 )   ( 5 )   ( 4 )   ( 3 )   ( 2 )   ( 1 )   ( 0 )
//  Output                                                           3
//  carry
//
//  1. Pre-calculation of P[i] and G[i]
//       cell | i |:
//         P[i] = A[i] xor B[i]
//         G[i] = A[i] and B[i]
//
//  2. Calculation of all carries in prefix tree (there are groups of cells at
//     each level. For example, at the level 1 there are 2 groups consisting 
//     of 1 cell, and at the level 2 there is 1 groups consisting 2 cells. 
//     Levels of prefix tree has numbers 0, 1, 2, 3, 4, etc. The numbers of 
//     cells at each level is 0, 1, 2, 3, etc.
//       cell X:
//         level 0:
//           P[i,j] = P[i] and P[j]
//           G[i,j] = G[i] or ( G[j] and P[i] )
//         levels before the last level:
//           P[i,j] = P[i,k] and P[k-1,j]
//           G[i,j] = G[i,k] or ( G[k-1,j] and P[i, k] )
//      
//       cell O:
//         level 0:
//           G[i,j] = G[i] or ( G[-1] and P[i] )
//         levels before the last level:
//           G[i,j] = G[i,k] or ( G[k-1,j] and P[i, k] )
//         the last level:
//           G[i,-1] = G[i] or ( G[i-1,-1] and P[i] )
//
//  3. Generating the sum
//       cell ( i ):
//         S[i] =  P[i] xor G[i,-1]
//

FLibrary::Out ArithmeticLibrary::synthAdder(size_t outSize, 
                                            const In &in, 
                                            bool plusOne, 
                                            GNet &net) {
  assert(in.size() == 2);

  // Setting input identifiers
  auto x = in[0];
  auto y = in[1];

  if (outSize >= x.size() && outSize >= y.size()) {
    // x.size() and y.size() become equal to the maximum of them
    filling(x.size(), {y}, net);
    filling(y.size(), {x}, net);
  } else {
    // x.size() and y.size() become equal to the outSize
    filling(outSize, {x}, net);
    filling(outSize, {y}, net);
    x.resize(outSize);
    y.resize(outSize);
  }

  // Setting input signals
  const size_t sumSize = x.size();
  auto inG = Signal::always(net.addGate(plusOne ? GateSymbol::ONE : GateSymbol::ZERO));
  auto xWire = formInputSignals(sumSize, x);
  auto yWire = formInputSignals(sumSize, y);

  const bool needsCarryOut = outSize > sumSize;
  if (!needsCarryOut && sumSize == 1) {
    Out out(sumSize);
    auto temp = Signal::always(net.addGate(GateSymbol::XOR, {xWire[0], yWire[0]}));
    out[0] = net.addGate(GateSymbol::XOR, {inG, temp});
    return out;
  } else {
    // Pre-calculation of P[i] and G[i]
    const size_t cut = needsCarryOut ? 1 : 2;
    SignalList preP = formCarrySignals(sumSize, GateSymbol::XOR, xWire, yWire, net);
    SignalList preG = formCarrySignals(sumSize-cut+1, GateSymbol::AND, xWire, yWire, net);

    // Carry signals in prefix tree
    SignalTree p;
    SignalTree g;

    const size_t size = needsCarryOut ? sumSize + 1 : sumSize;
    // A new level of prefix tree is added for size: 4, 6, 10, 18, etc.
    const size_t treeDepth = size <= 3 ? 1 : floor(log2(size - 2)) + 1;

    // Level 0 of prefix tree
    // Cell indexes start with lastReg = 0 and firstReg = -1.
    // Step bitween cells is 2 for lastreg and 2 for firstReg.
    auto spread = Signal::always(net.addGate(GateSymbol::AND, {inG, preP[0]}));
    g[{0, -1}] = Signal::always(net.addGate(GateSymbol::OR, {preG[0], spread}));
    size_t lastReg = 2;
    int firstReg = 1;
    while (lastReg <= sumSize - cut) {
      p[{lastReg, firstReg}] = Signal::always(net.addGate(GateSymbol::AND, {preP[lastReg], preP[firstReg]}));
      auto spread = Signal::always(net.addGate(GateSymbol::AND, {preG[firstReg], preP[lastReg]}));
      g[{lastReg, firstReg}] = Signal::always(net.addGate(GateSymbol::OR, {preG[lastReg], spread}));
      lastReg += 2;
      firstReg += 2;
    }

    // Levels of prefix tree before the last level
    // Cell indexes start with lastReg = (2^level) and firstReg = -1.  
    // a) Transition to cell in the same group:
    //      step is 2 only for lastReg
    // b) Transition to cell in a new group:
    //      step is (2+2^level) for lastReg and (2^(level+1))
    // c) middelGeg is needed to find index k:
    //      k = middleReg-1
    //      For each group middleReg is constant.
    //      It start with 2^level and has step (2^(level+1)) (It changes only when there is the transition to a new group)     
    // d) Transition condition to a new group of cells:
    //      (2^(level-1)) is a multiple of (number of cell + 1)
    size_t middleReg;
    for (size_t i = 1; i < treeDepth; i++) {
      lastReg = 1 << i;
      firstReg = -1;
      middleReg = 1 << i;
      size_t j = 0;
      while (lastReg <= sumSize - cut) {
        if (firstReg != -1) {
          p[{lastReg, firstReg}] = Signal::always(net.addGate(GateSymbol::AND, {p[{lastReg, middleReg-1}], p[{middleReg-2,firstReg}]}));
        }
        auto spread = Signal::always(net.addGate(GateSymbol::AND, {g[{middleReg-2, firstReg}], p[{lastReg, middleReg-1}]}));
        g[{lastReg, firstReg}] = Signal::always(net.addGate(GateSymbol::OR, {g[{lastReg, middleReg-1}], spread}));
        lastReg += 2;
        if ((j + 1) % (1 << (i - 1)) == 0) {
          lastReg += 1 << i;
          firstReg += 1 << (i + 1);
          middleReg += 1 << (i + 1);
        }
        j++;
      }
    }

    // The last level of prefix tree
    // Cell indexes start with lastReg = 1 and firstReg = -1.
    // Step bitween cells is 2 for lastreg and frstReg remains constant.
    lastReg = 1;
    while (lastReg <= sumSize - cut) {
      auto spread = Signal::always(net.addGate(GateSymbol::AND, {g[{lastReg - 1, -1}], preP[lastReg]}));
      g[{lastReg, -1}] = Signal::always(net.addGate(GateSymbol::OR, {preG[lastReg], spread}));
      lastReg += 2;
    }

    // Generating the sum
    Out out(sumSize);
    out[0] = net.addGate(GateSymbol::XOR, {preP[0], inG});
    for (size_t i = 1; i < sumSize; i++) {
      out[i] = net.addGate(GateSymbol::XOR, {preP[i], g[{i-1, -1}]});
    }
    if (needsCarryOut) {
      out.push_back(net.addGate(GateSymbol::NOP, {g[{sumSize-1, -1}]}));
    }

    filling(outSize, {out}, net);

    return out;
  }

}

inline void ArithmeticLibrary::filling(size_t size, 
                                       GateIdList &in, 
                                       GNet &net) {
  while (size > in.size()) {
    in.push_back(net.addGate(GateSymbol::ZERO));
  }
}

inline FLibrary::SignalList ArithmeticLibrary::formInputSignals(size_t size, 
                                                                GateIdList in) {
  SignalList list(size);
  for (size_t i = 0; i < size; i++) {
    list[i] = (Signal::always(in[i]));
  }
  return list;
}

inline FLibrary::SignalList ArithmeticLibrary::formCarrySignals(size_t size, 
                                                                GateSymbol func, 
                                                                SignalList &xWire, 
                                                                SignalList &yWire, 
                                                                GNet &net) {
  SignalList list(size);
  for (size_t i = 0; i < size; i++) {
    list[i] = Signal::always(net.addGate(func, { xWire[i], yWire[i] }));
  }
  return list;
}

} // namespace eda::rtl::library
