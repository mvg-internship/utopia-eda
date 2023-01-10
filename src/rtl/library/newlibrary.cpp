iiiii#include "rtl/library/newlibrary.h"

#include <cassert>
#include <cmath>
#include <memory>

using namespace eda::base::model;
using namespace eda::gate::model;
using namespace eda::rtl::model;

namespace eda::rtl::library {

bool NewFLibrary::supports(FuncSymbol func) const {
  if (func == FuncSymbol::ADD || func == FuncSymbol::SUB)
    return true;
  return false;
}

FLibrary::Out NewFLibrary::synth(size_t outSize, const Value &value, GNet &net) {
  Out out(outSize);

  return out;
}

FLibrary::Out NewFLibrary::synth(size_t outSize, FuncSymbol func, const In &in, GNet &net) {
  switch (func) {
  case FuncSymbol::ADD:
    return synthAdd(outSize, in, net);
  case FuncSymbol::SUB:
    return synthSub(outSize, in, net);
  default:
    assert(false);
    return {};
  }
}

FLibrary::Out NewFLibrary::synth(const Out &out, const In &in, const SignalList &control, GNet &net) {

  return out;
}

FLibrary::Out NewFLibrary::alloc(size_t outSize, GNet &net) {
  Out out(outSize);
    for (size_t i = 0; i < out.size(); i++) {
      out[i] = net.newGate();
    }

   return out;
}

FLibrary::Out NewFLibrary::synthAdd(size_t outSize, const In &in, GNet &net) {
  return synthAdder(outSize, in, false, false, net);
}

FLibrary::Out NewFLibrary::synthSub(size_t outSize, const In &in, GNet &net) {
  const auto &x = in[0];
  const auto &y = in[1];

  assert(outSize == y.size());

  Out temp(outSize);
  for (size_t i = 0; i < temp.size(); i++) {
    auto yWire = Signal::always(y[i]);
    temp[i] = net.addGate(GateSymbol::NOT, {yWire});
  }

  return synthAdder(outSize, {x, temp}, true, false, net);
}

FLibrary::Out NewFLibrary::synthAdder(size_t outSize, const In &in, bool plusOne, bool needsCarryOut, GNet &net) {

  assert(in.size() == 2);

  const auto &x = in[0];
  const auto &y = in[1];

  assert(x.size() == y.size() && outSize == x.size());

  auto carryIn = net.addGate(plusOne ? GateSymbol::ONE : GateSymbol::ZERO, {});
  Out out;

  //input Signals
  auto Gin = Signal::always(carryIn);
  SignalList xWire;
  SignalList yWire;
  for(size_t i = 0; i < outSize; i++) {
    xWire.push_back(Signal::always(x[i]));
    yWire.push_back(Signal::always(y[i]));
  }
 
  if(outSize == 1 && !needsCarryOut) {
    auto temp = Signal::always(net.addGate(GateSymbol::XOR, {xWire[0], yWire[0]}));
    out.push_back(net.addGate(GateSymbol::XOR, {temp, Gin}));
  }
  else {
    const size_t cut = needsCarryOut ? 1 : 2;
    //A preparatory level
    SignalList P;
    SignalList G;
    for(size_t i = 0; i < outSize-cut+1; i++) {
      P.push_back(Signal::always(net.addGate(GateSymbol::OR, {xWire[i], yWire[i]})));
      G.push_back(Signal::always(net.addGate(GateSymbol::AND, {xWire[i], yWire[i]})));
    } 
 
    //Prefix Tree
    const size_t treeDepth = ceil(log2(outSize+1));
    int lastReg;
    size_t firstReg;
    size_t support;
    size_t middleReg;
    SignalTree p;
    SignalTree g;
    for (size_t i = 0; i < treeDepth; i++) {
      lastReg = -1;
      firstReg = pow(2, i) - 1;
      support = pow(2, i);
      middleReg = pow(2, i);
      size_t j = 0;
      while (firstReg <= outSize - cut) {
        if (j % support == 0) { //Is a preparatory level used?
          if (i == 0) { //Is it the 0 level of the tree?
            if (lastReg != -1) 
              p[{firstReg, lastReg}] = generate_P(P[firstReg], P[lastReg], net);//P[firstReg] & P[lastReg]
            g[{firstReg, lastReg}] = generate_G(G[firstReg], lastReg == -1 ? Gin : G[lastReg], P[firstReg], net);//G[firstReg] | (G[lastReg] & P[firstReg]) 
          }
          else {
            if (lastReg != -1) 
              p[{firstReg, lastReg}] = generate_P(P[firstReg], p[{firstReg-1, lastReg}], net);//P[firstReg] & p[firstReg-1, lastReg]
            g[{firstReg, lastReg}] = generate_G(G[firstReg], g[{firstReg-1, lastReg}], P[firstReg], net);//G[firstReg] | (g[firstReg-1, lastReg] & P[firstReg])  
          }
        }
        else {
          if (lastReg != -1) 
            p[{firstReg, lastReg}] = generate_P(p[{firstReg, middleReg-1}], p[{middleReg-2, lastReg}], net);//p[firstReg, middleReg-1] & p[middleReg-2, lastReg]
          g[{firstReg, lastReg}] = generate_G(g[{firstReg, middleReg-1}], g[{middleReg-2, lastReg}], p[{firstReg, middleReg-1}], net);//g[firstReg, middleReg-1] | (g[middleReg-2, lastReg] & p[firstReg, middleReg-1])
        }
        firstReg += 1;
        if ((j + 1) % support == 0) {
          lastReg += pow(2, i + 1);
          firstReg += support;
          middleReg += pow(2, i + 1);
        }
        j++;
      }
    }

    //S[0] = (A[0]+B[0]) + Gin
    auto temporary = Signal::always(net.addGate(GateSymbol::XOR, {xWire[0], yWire[0]}));
    out.push_back(net.addGate(GateSymbol::XOR, {temporary, Gin}));
    for (size_t i = 1; i < outSize; i++) {
      auto temp = Signal::always(net.addGate(GateSymbol::XOR, {xWire[i], yWire[i]}));
      out.push_back(net.addGate(GateSymbol::XOR, {temp, g[{i-1, -1}]}));
    }
    if (needsCarryOut)
      out.push_back(net.addGate(GateSymbol::NOP, {g[{outSize-1, -1}]}));
  }

  return out;
}    

FLibrary::Signal NewFLibrary::generate_G(Signal &G, Signal &Gin, Signal &P, GNet &net) {
  auto spread = Signal::always(net.addGate(GateSymbol::AND, {Gin, P}));
  return Signal::always(net.addGate(GateSymbol::OR, {G, spread}));
}

FLibrary::Signal NewFLibrary::generate_P(Signal &P1, Signal &P2, GNet &net) {
  return Signal::always(net.addGate(GateSymbol::AND, {P1, P2}));
}

} //namespace eda::rtl::library
