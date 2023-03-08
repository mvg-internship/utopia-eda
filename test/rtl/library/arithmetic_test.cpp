//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "rtl/library/arithmetic.h"
#include "gate/simulator/simulator.h"

#include "gtest/gtest.h"

#include <cassert>
#include <cstdint>
#include <iostream>

using GNet = eda::gate::model::GNet;
using FuncSymbol = eda::rtl::model::FuncSymbol;
using FLibrary = eda::rtl::library::FLibrary;
using ArithmeticLibrary = eda::rtl::library::ArithmeticLibrary;
using Simulator = eda::gate::simulator::Simulator;
using BV = eda::gate::simulator::Simulator::Compiled::BV;

// Add inputs to the net and returns their identifiers
GNet::GateIdList addInputs(const size_t quantity,
                           GNet &net) {
  GNet::GateIdList list(quantity);
  for (size_t i = 0; i < quantity; i++) {
    list[i] = net.addIn();
  }
  return list;
}

// Add the links to the passed list
void addLinks(const GNet::GateIdList &ids,
              GNet::LinkList &links) {
  for (size_t i = 0; i < ids.size(); i++ ) {
    links.push_back(GNet::Link(ids[i]));
  }
}

// Generate number for BV and convert it to uint64_t
// inputReg - required number register
// maxReg - max register for significant figures
// (it is necessary to not overflow output register)
uint64_t generateInput(const size_t inputReg,
                       const size_t maxReg,
                       BV &input) {
  const size_t begin{input.size()};
  uint64_t number{0};
  for (size_t i = 0; i < inputReg; i++) {
    if (i < maxReg) {
      input.push_back(rand() % 2);
    } else {
      input.push_back(0);
    }
    number += input[begin + i] * (1ull << i);
  }
  return number;
}

// Convert from BV to uint64_t
uint64_t convertToInteger(const BV &vector) {
  uint64_t number{0};
  for (size_t i = 0; i < vector.size(); i++) {
    number += vector[i] * (1ull << i);
  }
  return number;
}

bool arithmeticTest(FuncSymbol func,
                    const size_t xSize,
                    const size_t ySize,
                    const size_t outSize) {
  GNet net;
  GNet::GateIdList x = addInputs(xSize, net);
  GNet::GateIdList y = addInputs(ySize, net);

  GNet::In inputs = {x, y};
  FLibrary &library = ArithmeticLibrary::get();
  GNet::GateIdList outputs = library.synth(outSize, func, inputs, net);
  net.sortTopologically();

  GNet::LinkList in;
  addLinks(x, in);
  addLinks(y, in);
  GNet::LinkList out;
  addLinks(outputs, out);

  Simulator simulator;
  auto compiled = simulator.compile(net, in, out);

  BV i;
  uint64_t first{0};
  uint64_t second{0};
  switch (func) {
  case FuncSymbol::ADD:
    first = generateInput(xSize, outSize - 1, i);
    second = generateInput(ySize, outSize - 1, i); 
    break;
  case FuncSymbol::SUB:
    do {
      first = generateInput(xSize, outSize, i);
      second = generateInput(ySize, outSize, i);
      if (second > first) {
        i.resize(0);
      }
    } while (i.size()==0);
    break; 
  default:
    assert(false);
  }
  BV o(outSize);

  compiled.simulate(o, i);

  uint64_t result{convertToInteger(o)};
  switch (func) {
  case FuncSymbol::ADD:
    return(result == first + second);
  case FuncSymbol::SUB:
    return(result == first - second);
  default:
    assert(false);
  return {};
  }
}

// Randomly generate a register from 1 to 64
size_t reg() {
  return 1 + rand() % 64;
}

TEST(arithmeticTest, addTest) {
  for (size_t i = 0; i < 100; i++) {
    assert(arithmeticTest(FuncSymbol::ADD, reg(), reg(), reg()));
  }
  EXPECT_TRUE(true);
}

TEST(arithmeticTest, subTest) {
  for (size_t i = 0; i < 10; i++) {
    assert(arithmeticTest(FuncSymbol::SUB, 10, 10, 11));
  }
  EXPECT_TRUE(true);
}
