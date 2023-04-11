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
uint64_t generateInput(const size_t inputDigit,
                       BV &input) {
  const size_t begin{input.size()};
  uint64_t number{0};
  for (size_t i = 0; i < inputDigit; i++) {
    input.push_back(rand() % 2);
    number += input[begin + i] * (1ull << i);
  }
  return number;
}

// Convert from integer to BV
BV convertToBV(uint64_t number) {
  BV vector;
  do {
    vector.push_back(number % 2);
    number = number / 2;
  } while (number != 0);
  return vector;
}

bool arithmeticTest(const FuncSymbol func,
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
  do {
    first = generateInput(xSize, in);
    second = generateInput(ySize, in);
    if ((second > first) && (func == FuncSymbol::SUB)) {
      in.resize(0);
    }
  } while (in.size() == 0);

  BV out(outSize);
  compiled.simulate(out, in);

  BV result; 
  switch (func) {
  case FuncSymbol::ADD:
    result = convertToBV(first + second);
    break;
  case FuncSymbol::SUB:
    result = convertToBV(first - second);
    break;
  case FuncSymbol::MUL:
    result = convertToBV(first * second);
    break;
  default:
    assert(false && "There are not tests fot this operation");
    return {};
  }
  bool flag = true;
  size_t checkSize = out.size() <= result.size() ? out.size() : result.size();
  for(size_t i = 0; i < checkSize; i++) {
    flag *= (out[i] == result[i]);
  }
  return flag;
}

// Randomly generate a digit from 1 to 64
size_t getDigit() {
  return 1 + rand() % 64;
}

TEST(ArithmeticTest, AddTest) {
  bool flag = true;
  for (size_t i = 0; i < 100; i++) {
    flag *= arithmeticTest(FuncSymbol::ADD, getDigit(), getDigit(), getDigit());
  }
  EXPECT_TRUE(flag);
}

TEST(ArithmeticTest, SubTest) {
  bool flag = true;
  for (size_t i = 0; i < 10; i++) {
    flag *= arithmeticTest(FuncSymbol::SUB, 10, 10, 11);
  }
  EXPECT_TRUE(flag);
}


TEST(ArithmeticTest, MulTest) {
  bool flag = true;
  for (size_t i = 0; i < 50; i++) {
    flag *= arithmeticTest(FuncSymbol::MUL, getDigit(), getDigit(), getDigit());
  }
  EXPECT_TRUE(flag);
}
