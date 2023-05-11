//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "gate/model/gnet.h"

#include <cassert>
#include <functional>
#include <vector>

namespace eda::gate::simulator {

/**
 * \brief Implements a simple simulator of (synchronous) gate-level nets.
 * \author <a href="mailto:kamkin@ispras.ru">Alexander Kamkin</a>
 */
class Simulator final {
  using Gate = eda::gate::model::Gate;
  using GNet = eda::gate::model::GNet;

public:
  /// Representation of a gate-level net optimized for simulation.
  class Compiled final {
    friend class Simulator;

    Compiled(const GNet &net,
             const GNet::LinkList &in,
             const GNet::LinkList &out);

  public:
    using B  = bool;
    using BV = std::vector<B>;
    using I  = std::size_t;
    using IV = std::vector<I>;

    /// Returns the number of inputs.
    I nSources() const { return nInputs; }
    /// Returns the number of outputs.
    I nTargets() const { return outputs.size(); }

    /// Evaluates the outputs from the inputs.
    template <typename T = BV>
    void simulate(T &out, const T &in) { 
      setTriggers();
      setSources(in);
      execute();
      getTargets(out);
    }

  private:
    /// Sets the input values.
    void setSources(const BV &values) {
      assert(values.size() == nInputs);
      for (I i = 0; i < nInputs; i++) {
        memory[i] = values[i];
      }
    }

    /// Sets the input values.
    void setSources(std::uint64_t values) {
      assert(nInputs <= 64);
      for (I i = 0; i < nInputs; i++) {
        memory[i] = (values >> i) & 1;
      }
    }

    /// Executes the postponed assignments.
    void setTriggers() {
      while (nPostponed > 0) {
        const auto assign = postponed[--nPostponed];
        const auto lhs = assign.first;
        const auto rhs = assign.second;

        memory[lhs] = rhs;
      }
    }

    /// Gets the output values.
    void getTargets(BV &values) {
      assert(values.size() == outputs.size());
      for (I i = 0; i < outputs.size(); i++) {
        values[i] = memory[outputs[i]];
      }
    }

    /// Gets the output values.
    void getTargets(std::uint64_t &values) {
      assert(outputs.size() <= 64);
      values = 0;
      for (I i = 0; i < outputs.size(); i++) {
        values |= (memory[outputs[i]] << i);
      }
    }

    /// Executes the compiled program.
    void execute() {
      for (auto &command : program) {
        command.op(command.out, command.in);
      }
    }

    /// Gate function (operation).
    using OP = std::function<void(I, IV)>;

    /// Single command.
    struct Command final {
      OP op;  // Operation.
      I  out; // Output.
      IV in;  // Inputs.
    };

    /// Returns the gate function.
    OP getOp(const Gate &gate) const;
    /// Construct the command for the given gate.
    Command getCommand(const GNet &net, const Gate &gate) const;

    /// Compiled program for the given net.
    std::vector<Command> program;
    /// Number of the program inputs.
    I nInputs;
    /// Program outputs: indices in memory (see below).
    IV outputs;

    /// Holds the state: first, inputs; then, internal gates.
    BV memory;

    /// Postponed assignments (for triggers).
    std::vector<std::pair<I, B>> postponed;
    /// Number of postponed assignments.
    I nPostponed;

    /// Maps source links and gates to memory indices.
    std::unordered_map<Gate::Link, I> gindex;

    //------------------------------------------------------------------------//
    // ZERO
    //------------------------------------------------------------------------//

    const OP opZero = [this](I out, IV in) {
      memory[out] = 0;
    };

    OP getZero(I arity) const { return opZero; }

    //------------------------------------------------------------------------//
    // ONE
    //------------------------------------------------------------------------//

    const OP opOne = [this](I out, IV in) {
      memory[out] = 1;
    };

    OP getOne(I arity) const { return opOne; }

    //------------------------------------------------------------------------//
    // NOP
    //------------------------------------------------------------------------//

    const OP opNop = [this](I out, IV in) {
      memory[out] = memory[in[0]];
    };

    OP getNop(I arity) const { return opNop; }

    //------------------------------------------------------------------------//
    // NOT
    //------------------------------------------------------------------------//

    const OP opNot = [this](I out, IV in) {
      memory[out] = !memory[in[0]];
    };

    OP getNot(I arity) const { return opNot; }

    //------------------------------------------------------------------------//
    // AND
    //------------------------------------------------------------------------//

    const OP opAnd2 = [this](I out, IV in) {
      memory[out] = memory[in[0]] && memory[in[1]];
    };

    const OP opAnd3 = [this](I out, IV in) {
      memory[out] = memory[in[0]] && memory[in[1]] && memory[in[2]];
    };

    const OP opAndN = [this](I out, IV in) {
      for (auto i : in) {
        if (!memory[i]) {
          memory[out] = 0;
          return;
        }
      }
      memory[out] = 1;
    };

    OP getAnd(I arity) const {
      switch (arity) {
      case  1: return opNop;
      case  2: return opAnd2;
      case  3: return opAnd3;
      default: return opAndN;
      }
    }

    //------------------------------------------------------------------------//
    // OR
    //------------------------------------------------------------------------//

    const OP opOr2 = [this](I out, IV in) {
      memory[out] = memory[in[0]] || memory[in[1]];
    };

    const OP opOr3 = [this](I out, IV in) {
      memory[out] = memory[in[0]] || memory[in[1]] || memory[in[2]];
    };

    const OP opOrN = [this](I out, IV in) {
      for (auto i : in) {
        if (memory[i]) {
          memory[out] = 1;
          return;
        }
      }
      memory[out] = 0;
    };

    OP getOr(I arity) const {
      switch (arity) {
      case  1: return opNop;
      case  2: return opOr2;
      case  3: return opOr3;
      default: return opOrN;
      }
    }

    //------------------------------------------------------------------------//
    // XOR
    //------------------------------------------------------------------------//

    const OP opXor2 = [this](I out, IV in) {
      memory[out] = memory[in[0]] ^ memory[in[1]];
    };

    const OP opXor3 = [this](I out, IV in) {
      memory[out] = memory[in[0]] ^ memory[in[1]] ^ memory[in[2]];
    };

    const OP opXorN = [this](I out, IV in) {
      bool result = 0;
      for (auto i : in) {
        result ^= memory[i];
      }
      memory[out] = result;
    };

    OP getXor(I arity) const {
      switch (arity) {
      case  1: return opNop;
      case  2: return opXor2;
      case  3: return opXor3;
      default: return opXorN;
      }
    }

    //------------------------------------------------------------------------//
    // NAND
    //------------------------------------------------------------------------//

    const OP opNand2 = [this](I out, IV in) {
      memory[out] = !(memory[in[0]] && memory[in[1]]);
    };

    const OP opNand3 = [this](I out, IV in) {
      memory[out] = !(memory[in[0]] && memory[in[1]] && memory[in[2]]);
    };

    const OP opNandN = [this](I out, IV in) {
      for (auto i : in) {
        if (!memory[i]) {
          memory[out] = 1;
          return;
        }
      }
      memory[out] = 0;
    };

    OP getNand(I arity) const {
      switch (arity) {
      case  1: return opNot;
      case  2: return opNand2;
      case  3: return opNand3;
      default: return opNandN;
      }
    }

    //------------------------------------------------------------------------//
    // NOR
    //------------------------------------------------------------------------//

    const OP opNor2 = [this](I out, IV in) {
      memory[out] = !(memory[in[0]] || memory[in[1]]);
    };

    const OP opNor3 = [this](I out, IV in) {
      memory[out] = !(memory[in[0]] || memory[in[1]] || memory[in[2]]);
    };

    const OP opNorN = [this](I out, IV in) {
      for (auto i : in) {
        if (memory[i]) {
          memory[out] = 0;
          return;
        }
      }
      memory[out] = 1;
    };

    OP getNor(I arity) const {
      switch (arity) {
      case  1: return opNot;
      case  2: return opNor2;
      case  3: return opNor3;
      default: return opNorN;
      }
    }

    //------------------------------------------------------------------------//
    // XNOR
    //------------------------------------------------------------------------//

    const OP opXnor2 = [this](I out, IV in) {
      memory[out] = memory[in[0]] == memory[in[1]];
    };

    const OP opXnor3 = [this](I out, IV in) {
      memory[out] = !(memory[in[0]] ^ memory[in[1]] ^ memory[in[2]]);
    };

    const OP opXnorN = [this](I out, IV in) {
      bool result = 1;
      for (auto i : in) {
        result ^= memory[i];
      }
      memory[out] = result;
    };

    OP getXnor(I arity) const {
      switch (arity) {
      case  1: return opNot;
      case  2: return opXnor2;
      case  3: return opXnor3;
      default: return opXnorN;
      }
    }

    //------------------------------------------------------------------------//
    // MAJ
    //------------------------------------------------------------------------//

    const OP opMaj3 = [this](I out, IV in) {
      memory[out] = (memory[in[0]] + memory[in[1]] + memory[in[2]]) >= 2;
    };

    const OP opMajN = [this](I out, IV in) {
      const size_t n = in.size();
      const size_t k = (n >> 1);

      unsigned w = memory[in[n - 1]];
      for (size_t i = 0; i < k; i++) {
        w += memory[in[(i << 1)]];
        w += memory[in[(i << 1)|1]];
      }

      memory[out] = (w > k);
    };

    OP getMaj(I arity) const {
      switch (arity) {
      case  1: return opNop;
      case  3: return opMaj3;
      default: return opMajN;
      }
    }

    //------------------------------------------------------------------------//
    // LATCH
    //------------------------------------------------------------------------//

    const OP opLatch = [this](I out, IV in) {
      const bool ena = memory[in[1]];
      if (ena) {
        postponed[nPostponed++] = {out, memory[in[0]]};
      }
    };

    OP getLatch(I arity) const { return opLatch; }

    //------------------------------------------------------------------------//
    // DFF
    //------------------------------------------------------------------------//

    const OP opDff = [this](I out, IV in) {
      // TODO: posedge(clk).
      const bool clk = memory[in[1]];
      if (clk) {
        postponed[nPostponed++] = {out, memory[in[0]]};
      }
    };

    OP getDff(I arity) const { return opDff; }

    //------------------------------------------------------------------------//
    // DFFrs
    //------------------------------------------------------------------------//

    const OP opDffrs = [this](I out, IV in) {
      // TODO: posedge(clk).
      const bool clk = memory[in[1]];
      const bool rst = memory[in[2]];
      const bool set = memory[in[3]];
      assert(!(rst && set));

      if (rst) {
        postponed[nPostponed++] = {out, 0};
      } else if (set) {
        postponed[nPostponed++] = {out, 1};
      } else if (clk) {
        postponed[nPostponed++] = {out, memory[in[0]]};
      }
    };

    OP getDffrs(I arity) const { return opDffrs; }
  };

  /// Compiles the given net.
  Compiled compile(const GNet &net,
                   const GNet::LinkList &in,
                   const GNet::LinkList &out) {
    return Compiled(net, in, out);
  }
};

} // namespace eda::gate::simulator
