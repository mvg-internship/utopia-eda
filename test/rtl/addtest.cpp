#include "gate/model/gnet.h"
#include "rtl/library/flibrary.h"
#include "gate/simulator/simulator.h"
#include "gtest/gtest.h"

#include <iostream>

using GNet = eda::gate::model::GNet;
using FuncSymbol = eda::rtl::model::FuncSymbol;
using FLibrary = eda::rtl::library::FLibrary;
using FLibraryDefault = eda::rtl::library::FLibraryDefault;
using simulator = eda::gate::simulator::Simulator;

bool addTest(FuncSymbol func, const size_t size) {

  GNet::In inputs(2);
  auto &term1 = inputs[0];
  auto &term2 = inputs[1];
  GNet net;

  for (size_t n = 0; n < size; n++) {
    term1.push_back(net.addIn());
  }

  for (size_t n = 0; n < size; n++) {
    term2.push_back(net.addIn());
  }

  FLibrary &library = FLibraryDefault::get();
  GNet::GateIdList outputs = library.synth(size, func, inputs, net);
  net.sortTopologically();
  GNet::LinkList in;

  for (size_t n = 0; n < size; n++) {
    in.push_back(GNet::Link(term1[n]));
  }
  for (size_t n = 0; n < size; n++) {
    in.push_back(GNet::Link(term2[n]));
  }

  GNet::LinkList out;

  for (size_t n = 0; n < size; n++) {
    out.push_back(Gate::Link(outputs[n]));
  }

  eda::gate::simulator::Simulator simulator;
  auto compiled = simulator.compile(net, in, out);

  std::vector<bool> output(size);
  std::vector<bool> input(size + size);

  for (size_t n = 0; n < size + size; n++) {
    input[n] = rand() % 2;
  }

  compiled.simulate(output, input);

  std::cout << "STARTING ADDER TEST" << std::endl;

  for (int n = size - 1; n >= 0; n--) {
    std::cout << input[n];
  }
  std::cout << "\n + \n";
  for (int n = size + size - 1; n >= term1.size(); n--) {
    std::cout << input[n];
  }
  std::cout << "\n = " << std::endl;

  for(int n = outputs.size() - 1; n >= 0; n--) {
    std::cout << output[n];
  }
  std::cout << std::endl;
  std::cout << "ADDER TEST HAS BEEN FINISHED" << std::endl;
 
  return true;
}

TEST(FLibraryDefaultTest, AddTest) {
  EXPECT_TRUE(addTest(FuncSymbol::ADD,  6));
}


