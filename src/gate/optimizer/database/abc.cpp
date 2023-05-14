//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gnet.h"
#include "gate/model/utils.h"
#include "gate/optimizer/rwdatabase.h"
#include "gate/optimizer/ttbuilder.h"
#include "util/math.h"

#include <algorithm>
#include <cstdlib>
#include <unordered_set>
#include <vector>

extern unsigned short s_RwrPracticalClasses[];
extern unsigned short s_RwtAigSubgraphs[];

namespace eda::gate::optimizer {

using Gate = eda::gate::model::Gate;
using GNet = eda::gate::model::GNet;
using GateId = Gate::Id;
using GateIdMap = GNet::GateIdMap;
using InputId = RWDatabase::InputId;
using TruthTable = RWDatabase::TruthTable;
using GateBindings = RWDatabase::GateBindings;
using BoundGNet = RWDatabase::BoundGNet;

/// The code below is based on ABC.
static BoundGNet getAbcNet(std::vector<std::pair<GateId, TruthTable>> &gates) {
  const unsigned short *graph = s_RwtAigSubgraphs;

  auto net = std::make_shared<GNet>();

  gates.push_back({net->addZero(), 0x0000}); // Constant 0
  gates.push_back({net->addIn(),   0xAAAA}); // Variable x0
  gates.push_back({net->addIn(),   0xCCCC}); // Variable x1
  gates.push_back({net->addIn(),   0xF0F0}); // Variable x2
  gates.push_back({net->addIn(),   0xFF00}); // Varaible x3

  GateBindings bindings;
  bindings[0] = gates[1].first;
  bindings[1] = gates[2].first;
  bindings[2] = gates[3].first;
  bindings[3] = gates[4].first;

  // Reconstruct the forest.
  for (size_t i = 0;; i++) {
    unsigned entry0 = graph[2*i + 0];
    unsigned entry1 = graph[2*i + 1];

    if (entry0 == 0 && entry1 == 0) {
      break;
    }

    // Get XOR flag.
    bool isXor = (entry0 & 1);
    entry0 >>= 1;

    // Get the nodes.
    const auto &[gid0, table0] = gates[entry0 >> 1];
    const auto &[gid1, table1] = gates[entry1 >> 1];

    const auto not0 = (entry0 & 1);
    const auto not1 = (entry1 & 1);

    const auto i0 = not0 ? net->addNot(gid0) : gid0;
    const auto i1 = not1 ? net->addNot(gid1) : gid1;
    const auto f0 = not0 ? ~table0 : table0;
    const auto f1 = not1 ? ~table1 : table1;

    const auto gid = (isXor ? net->addXor(i0, i1) : net->addAnd(i0, i1));
    const auto table = (isXor ? (f0 ^ f1) : (f0 & f1)) & 0xFFFF;
    gates.push_back({gid, table});
  }

  return BoundGNet{net, bindings};
}

static BoundGNet getCircuit(GateId gid, const BoundGNet &bnet) {
  std::vector<GateId> gates;
  gates.push_back(gid);

  for (size_t i = 0; i < gates.size(); i++) {
    const auto *gate = Gate::get(gates[i]);

    for (auto input : gate->inputs()) {
      const auto inputId = input.node();
      gates.push_back(inputId);
    }
  }

  auto circuit = std::make_shared<GNet>();

  GateIdMap oldToNewGates;
  for (auto i = gates.rbegin(); i != gates.rend(); i++) {
    if (oldToNewGates.find(*i) == oldToNewGates.end()) {
      const auto *gate = Gate::get(*i);
      const auto newInputs = model::getNewInputs(gate->inputs(), oldToNewGates);
      const auto newGateId = circuit->addGate(gate->func(), newInputs);
      oldToNewGates[*i] = newGateId;
    }
  }

  circuit->addOut(oldToNewGates[gid]);

  GateBindings bindings;
  InputId inputId = 0;
  for (const auto &[id, gid] : bnet.bindings) {
    if (oldToNewGates.find(gid) != oldToNewGates.end()) {
      bindings[inputId++ /* No holes */] = oldToNewGates[gid];
    }
  }

  return BoundGNet{circuit, bindings};
}

static BoundGNet clone(const BoundGNet &circuit) {
  BoundGNet newCircuit;

  GateIdMap oldToNewGates;
  newCircuit.net = std::shared_ptr<GNet>(circuit.net->clone(oldToNewGates));

  for (const auto &[inputId, oldGateId] : circuit.bindings) {
    const auto newGateId = oldToNewGates[oldGateId];
    assert(newGateId != 0);

    newCircuit.bindings[inputId] = newGateId;
//    std::cout << "inputId " << inputId << "->" << newGateId << std::endl;
  }

  return newCircuit;
}

static void generateNpnInstances(const BoundGNet &bnet, RWDatabase &database) {
  static size_t perm[24][4] = {
    {0, 1, 2, 3},
    {1, 0, 2, 3},
    {2, 0, 1, 3},
    {0, 2, 1, 3},
    {1, 2, 0, 3},
    {2, 1, 0, 3},
    {2, 1, 3, 0},
    {1, 2, 3, 0},
    {3, 2, 1, 0},
    {2, 3, 1, 0},
    {1, 3, 2, 0},
    {3, 1, 2, 0},
    {3, 0, 2, 1},
    {0, 3, 2, 1},
    {2, 3, 0, 1},
    {3, 2, 0, 1},
    {0, 2, 3, 1},
    {2, 0, 3, 1},
    {1, 0, 3, 2},
    {0, 1, 3, 2},
    {3, 1, 0, 2},
    {1, 3, 0, 2},
    {0, 3, 1, 2},
    {3, 0, 1, 2}
  };

  const size_t k = bnet.bindings.size();
  const size_t N = 1 << (k + 1);
  const size_t P = utils::factorial(k);

  for (unsigned n = 0; n < N; n++) {
    // Clone the net consistently w/ the bindings.
    auto circuit = clone(bnet);

    // Negate the inputs.
    for (unsigned i = 0; i < k; i++) {
      if ((n >> i) & 1) {
        auto oldInputId = circuit.bindings[i];
        auto newInputId = circuit.net->addIn();
        circuit.net->setNot(oldInputId, newInputId);
        circuit.bindings[i] = newInputId;
      }
    }

    // Negate the output.
    if ((n >> k) & 1) {
      auto oldOutputId = circuit.net->targetLinks().begin()->source;
      auto *gate = Gate::get(oldOutputId);
      circuit.net->setNot(oldOutputId, gate->input(0));
      circuit.net->addOut(oldOutputId);
    }

    // Permute the inputs.
    for (unsigned p = 0; p < P; p++) {
      for (unsigned i = 0; i < k; i++) {
        const auto j = perm[p][i];

        if (i < j) {
          const auto tempGate = circuit.bindings[i];
          circuit.bindings[i] = circuit.bindings[j];
          circuit.bindings[j] = tempGate;
        }
      }

      const auto truthTable = TTBuilder::build(circuit);
//      std::cout << "Table: " << std::hex << truthTable << std::endl;
//      std::cout << std::dec << *circuit.net << std::endl;

      database.set(truthTable, {circuit} /* One circuit per truth table */);
    }
  }
}

void initializeAbcRwDatabase(RWDatabase &database) {
  const size_t maxNpn4 = 222;
  const unsigned short *classes = s_RwrPracticalClasses;

  std::vector<std::pair<GateId, TruthTable>> gates;
  auto net = getAbcNet(gates);

  std::unordered_set<TruthTable> practicalNpnClasses;
  practicalNpnClasses.reserve(maxNpn4);
  practicalNpnClasses.emplace(0);
  for (size_t i = 1; classes[i]; i++) {
    practicalNpnClasses.emplace(classes[i]);
  }

  std::unordered_set<TruthTable> processedNpnClasses;
  processedNpnClasses.reserve(practicalNpnClasses.size());

  for (const auto &[gid, truthTable] : gates) {
    if (practicalNpnClasses.find(truthTable) != practicalNpnClasses.end() &&
        processedNpnClasses.find(truthTable) == processedNpnClasses.end()) {
      auto circuit = getCircuit(gid, net);
      generateNpnInstances(circuit, database);

      // ABC truth tables are used only for filtering.
      processedNpnClasses.emplace(truthTable);
    }
  }
}

} // namespace eda::gate::optimizer
