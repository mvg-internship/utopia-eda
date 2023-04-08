#include "gate/model/gnet.h"
#include "rtl/library/flibrary.h"

using FLibrary = eda::rtl::library::FLibrary;
using GateIdList = FLibrary::GateIdList;
using GateSymbol = eda::gate::model::GateSymbol;
using GNet = eda::gate::model::GNet;

namespace eda::rtl::library {

// Complete GateIdList with zeros up to the passed size:
// 111 -> 000111
void fillingWithZeros(const size_t size,
                      GateIdList &in,
                      GNet &net) {
  while (size > in.size()) {
    in.push_back(net.addZero());
  }
}

// Form GateIdList of outputs for the operation
// applied to pairs of input identifiers
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

// Make inputs equal to each other,
// but no longer than outsize
void makeInputsEqual(const size_t outSize,
                     GateIdList &x,
                     GateIdList &y,
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

// Divide one GaieIdList into two GateIdLists:
// 111000 -> 111 and 000 (for firstPartSize = 3)
void getPartsOfGateIdList(const GateIdList &x,
                          GateIdList &x1,
                          GateIdList &x0,
                          const size_t firstPartSize) {
  size_t i{0};
  for (i = 0; i < firstPartSize; i++) {
    x0.push_back(x[i]);
  }
  for (; i < x.size(); i++) {
    x1.push_back(x[i]);
  }
}

// Make left shift for GateIdList:
// 111 -> 111000 (for shift = 3)
GateIdList leftShiftForGateIdList(const GateIdList &x,
                                  const size_t shift,
                                  GNet &net) {
  GateIdList list(x.size() + shift);
  size_t i;
  for (i = 0; i < shift; i++) {
    list[i] = net.addZero();
  }
  for (; i < list.size(); i++) {
    list[i] = x[i - shift];
  }
  return list;
}

} // namespace eda::rtl::library
