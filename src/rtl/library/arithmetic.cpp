//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "rtl/library/arithmetic.h"
#include "rtl/model/fsymbol.h"

#include <cassert>
#include <cmath>
#include <iostream>

using GNet = eda::gate::model::GNet;
using GateId = GNet::GateId;
using GateIdList = eda::rtl::library::FLibrary::GateIdList;
using GateSymbol = eda::gate::model::GateSymbol;
using FuncSymbol = eda::rtl::model::FuncSymbol;

namespace eda::rtl::library {

// Complete GateIdList with zeros up to the passed size
void fillingWithZeros(const size_t size,
                      FLibrary::GateIdList &in,
                      GNet &net);

// Make inputs equal to each other,
// but no longer than outSize
inline void makeInputsEqual(const size_t outSize,
                            FLibrary::GateIdList &x,
                            FLibrary::GateIdList &y,
                            GNet &net);

// Form GateIdList of outputs for the operation
// applied to pairs of input identifiers
GateIdList formGateIdList(const size_t size,
                          GateSymbol func,
                          const GateIdList &x,
                          const GateIdList &y,
                          GNet &net);

//TODO In the future, ArithmeticLibrary will be responsible only
//for arithmetic operations
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
  auto x = in[0];
  auto y = in[1];

  makeInputsEqual(outSize, x, y, net);

  return synthAdder(outSize, {x, y}, false, net);
}

FLibrary::Out ArithmeticLibrary::synthSub(size_t outSize,
                                          const In &in,
                                          GNet &net) {
  auto x = in[0];
  auto y = in[1];

  makeInputsEqual(outSize, x, y, net);

  Out temp(y.size());
  for (size_t i = 0; i < temp.size(); i++) {
    temp[i] = net.addGate(GateSymbol::NOT, y[i]);
  }

  return synthAdder(outSize, {x, temp}, true, net);
}

//                            LADNER-FISHER'S ADDER
//
//  G - gates for generated carry
//  P - gates for propagated carry
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
//  1. Pre-calculation of P[i] and G[i]:
//       cell | i |:
//         P[i] = A[i] xor B[i]
//         G[i] = A[i] and B[i]
//
//  2. Calculation of all carries in the prefix tree (the prefix tree levels
//     (vertically from top to bottom) have numbers: 0, 1, 2, 3, 4, etc.
//     The cells numbers (horizontally from right to left) start with 0
//     at each level. There are groups of cells at each level. For example,
//     there are 2 groups consisting of 1 cell at the level #1
//     and there is 1 group consisting 2 cells at the level #2):
//       cell X:
//         the level #0:
//           P[i,j] = P[i] and P[j]
//           G[i,j] = G[i] or (P[i] and G[j])
//         levels before the last level:
//           P[i,j] = P[i,k] and P[k - 1,j]
//           G[i,j] = G[i,k] or (P[i,k] and G[k - 1,j])
//
//       cell O:
//         the level #0:
//           G[i,j] = G[i] or (P[i] and G[-1])
//         levels before the last level:
//           G[i,j] = G[i,k] or (P[i,k] and G[k - 1,j])
//         the last level:
//           G[i,-1] = G[i] or (P[i] and G[i - 1,-1])
//
//  3. Generating the sum:
//       cell ( i ):
//         S[i] =  P[i] xor G[i,-1]
//
FLibrary::Out ArithmeticLibrary::synthAdder(size_t outSize,
                                            const In &in,
                                            bool plusOne,
                                            GNet &net) {
  const auto &x = in[0];
  const auto &y = in[1];

  assert((x.size() == y.size()) && "Terms must be equal in size");

  auto carryIn = plusOne ? net.addOne() : net.addZero();
  const size_t inSize = x.size();
  const bool needsCarryOut = ((outSize > inSize) && (plusOne == 0));

  if ((!needsCarryOut) && (inSize == 1)) {
    Out out(1);
    auto temp = net.addGate(GateSymbol::XOR, x[0], y[0]);
    out[0] = net.addGate(GateSymbol::XOR, carryIn, temp);
    return out;
  } else {
    const size_t maxDigit = needsCarryOut ? (inSize - 1) : (inSize - 2);
    auto preP = formGateIdList(inSize, GateSymbol::XOR, x, y, net);
    auto preG = formGateIdList(maxDigit + 1, GateSymbol::AND, x, y, net);

    // The tree of gates for generated carry
    GateIdTree p;
    // The tree of gates for propagated carry
    GateIdTree g;

    const size_t size = needsCarryOut ? (inSize + 1) : inSize;
    // A new level of the prefix tree is added for size: 4, 6, 10, 18, etc.
    const size_t treeDepth = (size <= 3) ? 1 : (floor(log2(size - 2)) + 1);

    // The level #0 of the prefix tree
    // Cell indices start with lastDigit = 0 and firstDigit = -1.
    // Step between cells is 2 for lastDigit and 2 for firstDigit.
    auto temp = net.addGate(GateSymbol::AND, preP[0], carryIn);
    g[{0, -1}] = net.addGate(GateSymbol::OR, preG[0], temp);
    GateIdKey key(2, 1);
    size_t &lastDigit{key.first};
    int &firstDigit{key.second};
    while (lastDigit <= maxDigit) {
      p[key] = net.addGate(GateSymbol::AND, preP[lastDigit], preP[firstDigit]);
      temp = net.addGate(GateSymbol::AND, preP[lastDigit], preG[firstDigit]);
      g[key] = net.addGate(GateSymbol::OR, preG[lastDigit], temp);
      lastDigit += 2;
      firstDigit += 2;
    }

    // Levels of prefix tree before the last level
    // Cell indices start with lastDigit = (2^level) and firstDigit = -1.
    // a) Transition to cell in the same group:
    //      step is 2 only for lastDigit
    // b) Transition to cell in a new group:
    //      step is (2 + 2^level) for lastDigit and (2^(level + 1)) for firstDigit
    // c) middelDigit is needed to find index k:
    //      k = middleDigit - 1
    //      For each group of cells middleDigit is constant.
    //      It starts with 2^level and has step (2^(level + 1))
    //      (It changes only when there is the transition to a new group)
    // d) The condition of transition to a new group of cells:
    //      (cell + 1) is a multiple of (2^(level - 1))
    size_t middleDigit{0};
    std::pair<size_t&, size_t> lKey(lastDigit, 0);
    std::pair<size_t, int&> rKey(0, firstDigit);
    for (size_t i = 1; i < treeDepth; i++) {// i - the number of the level
      lastDigit = (1 << i);
      firstDigit = -1;
      middleDigit = (1 << i);
      lKey.second = middleDigit - 1;
      rKey.first = middleDigit - 2;
      size_t j{0};// j - the number of the cell
      while (lastDigit <= maxDigit) {
        if (firstDigit != -1) {
          p[key] = net.addGate(GateSymbol::AND, p[lKey], p[rKey]);
        }
        temp = net.addGate(GateSymbol::AND, p[lKey], g[rKey]);
        g[key] = net.addGate(GateSymbol::OR, g[lKey], temp);
        lastDigit += 2;
        if ((j + 1) % (1 << (i - 1)) == 0) {
          lastDigit += (1 << i);
          firstDigit += (1 << (i + 1));
          middleDigit += (1 << (i + 1));
          lKey.second = middleDigit - 1;
          rKey.first = middleDigit - 2;
        }
        j++;
      }
    }

    // The last level of prefix tree
    // Cell indices start with lastDigit = 1 and firstDigit = -1.
    // Step between cells is 2 for lastDigit but firstDigit remains constant.
    lastDigit = 1;
    firstDigit = -1;
    while (lastDigit <= maxDigit) {
      temp = net.addGate(GateSymbol::AND, preP[lastDigit], g[{lastDigit - 1, -1}]);
      g[key] = net.addGate(GateSymbol::OR, preG[lastDigit], temp);
      lastDigit += 2;
    }

    // Generating the sum
    Out out(inSize);
    out[0] = net.addGate(GateSymbol::XOR, preP[0], carryIn);
    for (size_t i = 1; i < inSize; i++) {
      out[i] = net.addGate(GateSymbol::XOR, preP[i], g[{i - 1, -1}]);
    }
    if (needsCarryOut) {
      out.push_back(net.addGate(GateSymbol::NOP, g[{inSize - 1, -1}]));
    }

    fillingWithZeros(outSize, {out}, net);

    return out;
  }

}

void fillingWithZeros(const size_t size,
                      FLibrary::GateIdList &in,
                      GNet &net) {
  while (size > in.size()) {
    in.push_back(net.addZero());
  }
}

void makeInputsEqual(const size_t outSize,
                     FLibrary::GateIdList &x,
                     FLibrary::GateIdList &y,
                     GNet &net) {
  if ((outSize >= x.size()) && (outSize >= y.size())) {
    // Make x.size() and y.size() equal to the maximum of them
    fillingWithZeros(x.size(), {y}, net);
    fillingWithZeros(y.size(), {x}, net);
  } else {
    // Make x.size() and y.size() equal to the outSize
    fillingWithZeros(outSize, {x}, net);
    fillingWithZeros(outSize, {y}, net);
    x.resize(outSize);
    y.resize(outSize);
  }
}

GateIdList formGateIdList(const size_t size,
                          GateSymbol func, 
                          const GateIdList &x,
                          const GateIdList &y,
                          GNet &net) {
  GateIdList list(size);
  for (size_t i = 0; i < size; i++) {
    list[i] = net.addGate(func, x[i], y[i]);
  }
  return list;
}

} // namespace eda::rtl::library
