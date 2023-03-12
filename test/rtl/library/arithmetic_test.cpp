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
  for (size_t i = 0; i < ids.size(); i++) {
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
  size_t i{0};
  for (i = 0; (i < inputReg) && (i < maxReg); i++) {
    input.push_back(rand() % 2);
    number += input[begin + i] * (1ull << i);
  }
  while (i < inputReg) { 
    input.push_back(0);
    i++;
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
  GNet::GateIdList xIds = addInputs(xSize, net);
  GNet::GateIdList yIds = addInputs(ySize, net);

  GNet::In inputIds = {xIds, yIds};
  FLibrary &library = ArithmeticLibrary::get();
  GNet::GateIdList outputIds = library.synth(outSize, func, inputIds, net);
  net.sortTopologically();

  GNet::LinkList inputLinks;
  addLinks(xIds, inputLinks);
  addLinks(yIds, inputLinks);
  GNet::LinkList outputLinks;
  addLinks(outputIds, outputLinks);

  Simulator simulator;
  auto compiled = simulator.compile(net, inputLinks, outputLinks);

  BV in;
  uint64_t first{0};
  uint64_t second{0};
  switch (func) {
  case FuncSymbol::ADD:
    first = generateInput(xSize, outSize - 1, in);
    second = generateInput(ySize, outSize - 1, in); 
    break;
  case FuncSymbol::SUB:
    do {
      first = generateInput(xSize, outSize, in);
      second = generateInput(ySize, outSize, in);
      if (second > first) {
        in.resize(0);
      }
    } while (in.size() == 0);
    break; 
  default:
    assert(false);
  }
  BV out(outSize);

  compiled.simulate(out, in);

  uint64_t result{convertToInteger(out)};
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

// Randomly generate a digit from 1 to 64
size_t getDigit() {
  return 1 + rand() % 64;
}

TEST(arithmeticTest, addTest) {
  for (size_t i = 0; i < 100; i++) {
    assert(arithmeticTest(FuncSymbol::ADD, getDigit(), getDigit(), getDigit()));
  }
  EXPECT_TRUE(true);
}

TEST(arithmeticTest, subTest) {
  for (size_t i = 0; i < 10; i++) {
    assert(arithmeticTest(FuncSymbol::SUB, 10, 10, 11));
  }
  EXPECT_TRUE(true);
}
