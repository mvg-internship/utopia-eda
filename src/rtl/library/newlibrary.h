#pragma once

#include "gate/model/gnet.h"
#include "rtl/library/flibrary.h"
#include "rtl/model/fsymbol.h"

#include <cassert>
#include <map>
#include <memory>
#include <utility>
#include <vector>

using namespace eda::gate::model;
using namespace eda::rtl::model;

namespace eda::rtl::library {
class NewFLibrary final: public FLibrary {
public:
  using Key = std::pair<int, int>;
  using SignalTree = std::map<Key, Signal>;

  static FLibrary &get() {
    static auto instance = std::unique_ptr<FLibrary>(new NewFLibrary());
    return *instance;
  }

  bool supports(FuncSymbol func) const override;

  Out synth(size_t outSize, const Value &value, GNet &net) override;

  Out synth(size_t outSize, FuncSymbol func, const In &in, GNet &net) override;

  Out synth(const Out &out, const In &in, const SignalList &control, GNet &net) override;

  Out alloc(size_t outSize, GNet &net) override;

private:
  NewFLibrary() {}
  ~NewFLibrary() override {}

  static Out synthAdd(size_t outSize, const In &in, GNet &net);

  static Out synthSub(size_t outSize, const In &in, GNet &net);

  static Out synthAdder(size_t outSize, const In &in, bool plusOne, bool needsCarryOut, GNet &net);

  static Signal generate_G(Signal &G, Signal &Gin, Signal &P, GNet &net);

  static Signal generate_P(Signal &P1, Signal &P2, GNet &net);

};

} // namespace eda::rtl::library

