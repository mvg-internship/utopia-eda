//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021-2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/checker.h"
#include "gate/debugger/encoder.h"
#include "gate/simulator/simulator.h"

#include <cassert>

namespace eda::gate::debugger {

bool Checker::areEqual(GNet &lhs,
                       GNet &rhs,
                       GateIdMap &gmap) {
  GateBinding ibind, obind, tbind;

  // Input-to-input correspondence.
  for (auto oldSourceLink : lhs.sourceLinks()) {
    auto newSourceId = gmap[oldSourceLink.target];
    ibind.insert({oldSourceLink, Gate::Link(newSourceId)});
  }

  // Output-to-output correspondence.
  for (auto oldTargetLink : lhs.targetLinks()) {
    auto newTargetId = gmap[oldTargetLink.source];
    obind.insert({oldTargetLink, Gate::Link(newTargetId)});
  }

  // Trigger-to-trigger correspondence.
  for (auto oldTriggerId : lhs.triggers()) {
    auto newTriggerId = gmap[oldTriggerId];
    tbind.insert({Gate::Link(oldTriggerId), Gate::Link(newTriggerId)});
  }

  Checker::Hints hints;
  hints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tbind));

  return areEqual(lhs, rhs, hints);
}

bool Checker::areEqual(const GNet &lhs,
                       const GNet &rhs,
                       const Hints &hints) const {
  const unsigned flatCheckBound = 64 * 1024;

  assert(hints.isKnownIoPortBinding());
  assert(lhs.nSourceLinks() == rhs.nSourceLinks());
  assert(lhs.nSourceLinks() <= hints.sourceBinding->size());
  assert(rhs.nTargetLinks() <= hints.targetBinding->size());

  if (hints.isKnownSubnetBinding() &&
      lhs.nGates() + rhs.nGates() > 2 * flatCheckBound) {
    return areEqualHier(lhs, rhs, hints);
  }

  assert(lhs.isComb() == rhs.isComb());

  if (lhs.isComb() && rhs.isComb()) {
    return areEqualComb(lhs, rhs,
                       *hints.sourceBinding,
                       *hints.targetBinding);
  }

  if (hints.isKnownTriggerBinding()) {
    return areEqualSeq(lhs, rhs,
                      *hints.sourceBinding,
                      *hints.targetBinding,
                      *hints.triggerBinding);
  }

  if (hints.isKnownStateEncoding()) {
    return areEqualSeq(lhs, rhs,
                      *hints.encoder,
                      *hints.decoder,
                      *hints.sourceBinding,
                      *hints.targetBinding,
                      *hints.lhsTriEncIn,
                      *hints.lhsTriDecOut,
                      *hints.rhsTriEncOut,
                      *hints.rhsTriDecIn);
  }

  assert(false && "Unimplemented LEC");
  return false;
}

bool Checker::areEqualHier(const GNet &lhs,
                           const GNet &rhs,
                           const Hints &hints) const {
  assert(!lhs.isFlat() && !rhs.isFlat());
  assert(lhs.nSubnets() == rhs.nSubnets());
  assert(lhs.nSubnets() == hints.subnetBinding->size());
  assert(hints.isKnownInnerBinding());

  for (const auto &[lhsSubnetId, rhsSubnetId] : *hints.subnetBinding) {
    const auto *lhsSubnet = lhs.subnet(lhsSubnetId);
    const auto *rhsSubnet = rhs.subnet(rhsSubnetId);


    GateBinding imap;
    for (auto lhsLink : lhsSubnet->sourceLinks()) {
      const auto &binding = lhs.hasSourceLink(lhsLink) ? *hints.sourceBinding
                                                       : *hints.innerBinding;
      auto i = binding.find(lhsLink);
      assert(i != binding.end());
      imap.insert({lhsLink, i->second});
    }

    GateBinding omap;
    for (auto lhsLink : lhsSubnet->targetLinks()) {
      const auto &binding = lhs.hasTargetLink(lhsLink) ? *hints.targetBinding
                                                       : *hints.innerBinding;
      auto i = binding.find(lhsLink);
      assert(i != binding.end());
      imap.insert({lhsLink, i->second});
    }

    Hints hintsSubnets;
    hintsSubnets.sourceBinding = std::make_shared<GateBinding>(std::move(imap));
    hintsSubnets.targetBinding = std::make_shared<GateBinding>(std::move(omap));
    hintsSubnets.innerBinding  = hints.innerBinding;

    if (!areEqual(*lhsSubnet, *rhsSubnet, hintsSubnets)) {
      return false;
    }
  }

  return true;
}

bool Checker::areEqualComb(const GNet &lhs,
                           const GNet &rhs,
                           const GateBinding &ibind,
                           const GateBinding &obind) const {
  const unsigned simCheckBound = 8;

  if (lhs.nSourceLinks() <= simCheckBound) {
    return areEqualCombSim(lhs, rhs, ibind, obind);
  }

  return areEqualCombSat({ &lhs, &rhs }, nullptr, ibind, obind);
}

bool Checker::areEqualSeq(const GNet &lhs,
                          const GNet &rhs,
                          const GateBinding &ibind,
                          const GateBinding &obind,
                          const GateBinding &tbind) const {
  GateBinding imap(ibind);
  GateBinding omap(obind);

  // Cut triggers.
  for (const auto &[lhsLink, rhsLink] : tbind) {
    const Gate *lhsTrigger = Gate::get(lhsLink.source);
    const Gate *rhsTrigger = Gate::get(rhsLink.source);

    assert(lhsTrigger->func()  == rhsTrigger->func());
    assert(lhsTrigger->arity() == rhsTrigger->arity());

    imap.insert({Gate::Link(lhsTrigger->id()), Gate::Link(rhsTrigger->id())});

    for (std::size_t i = 0; i < lhsTrigger->arity(); i++) {
      const Gate::Signal lhsInput = lhsTrigger->input(i);
      const Gate::Signal rhsInput = rhsTrigger->input(i);

      omap.insert({Gate::Link(lhsInput.node()), Gate::Link(rhsInput.node())});
    }
  }

  return areEqualComb(lhs, rhs, imap, omap);
}

bool Checker::areEqualSeq(const GNet &lhs,
                          const GNet &rhs,
                          const GNet &enc,
                          const GNet &dec,
                          const GateBinding &ibind,
                          const GateBinding &obind,
                          const GateBinding &lhsTriEncIn,
                          const GateBinding &lhsTriDecOut,
                          const GateBinding &rhsTriEncOut,
                          const GateBinding &rhsTriDecIn) const {
  
  //=========================================//
  //                                         //
  //   inputs---------inputs                 //
  //    LHS'           RHS'                  //
  //     |              |                    //
  //   encode           |                    //
  //     |--------------|---------- outputs' //
  // (triggers)     (triggers)               //
  //     |--------------|---------- inputs'  //
  //   decode           |                    //
  //     |              |                    //
  //    LHS''          RHS''                 //
  //  outputs--------outputs                 //
  //                                         //
  //=========================================//

  GateConnect connectTo;
  GateBinding imap(ibind);
  GateBinding omap(obind);

  // Connect the encoder inputs to the LHS-trigger D inputs' drivers.
  for (const auto &[lhsTriLink, encInLink] : lhsTriEncIn) {
    const auto *lhsTrigger = Gate::get(lhsTriLink.source);
    connectTo.insert({encInLink.source, lhsTrigger->input(0).node()});
  }

  // Connect the LHS-trigger outputs to the decoder outputs.
  for (const auto &[lhsTriLink, decOutLink] : lhsTriDecOut) {
    connectTo.insert({lhsTriLink.source, decOutLink.source});
  }

  // Append the encoder outputs and the RHS-trigger inputs to the outputs.
  for (const auto &[rhsTriLink, encOutLink] : rhsTriEncOut) {
    const auto *rhsTrigger = Gate::get(rhsTriLink.source);
    omap.insert({encOutLink, Gate::Link(rhsTrigger->input(0).node())});
  }

  // Append the decoder inputs and the RHS-trigger outputs to to the inputs.
  for (const auto &[rhsTriLink, decInLink] : rhsTriDecIn) {
    imap.insert({decInLink, rhsTriLink});
  }

  return areEqualCombSat({&lhs, &rhs, &enc, &dec}, &connectTo, imap, omap);
}

bool Checker::areEqualCombSim(const GNet &lhs,
                              const GNet &rhs,
                              const GateBinding &ibind,
                              const GateBinding &obind) const {
  assert(lhs.nSourceLinks() == rhs.nSourceLinks());
  assert(lhs.nSourceLinks() <= 16);

  GNet::LinkList lhsInputs;
  GNet::LinkList rhsInputs;

  lhsInputs.reserve(ibind.size());
  rhsInputs.reserve(ibind.size());

  for (const auto &[lhsLink, rhsLink] : ibind) {
    lhsInputs.push_back(lhsLink);
    rhsInputs.push_back(rhsLink);
  }

  GNet::LinkList lhsOutputs;
  GNet::LinkList rhsOutputs;

  lhsOutputs.reserve(obind.size());
  rhsOutputs.reserve(obind.size());

  for (const auto &[lhsLink, rhsLink] : obind) {
    lhsOutputs.push_back(lhsLink);
    rhsOutputs.push_back(rhsLink);
  }

  eda::gate::simulator::Simulator simulator;

  auto lhsCompiled = simulator.compile(lhs, lhsInputs, lhsOutputs);
  auto rhsCompiled = simulator.compile(rhs, rhsInputs, rhsOutputs);

  for (std::uint64_t in = 0; in < (1ull << lhs.nSourceLinks()); in++) {
    std::uint64_t lhsOut, rhsOut;

    lhsCompiled.simulate(lhsOut, in);
    rhsCompiled.simulate(rhsOut, in);

    if (lhsOut != rhsOut) {
      return false;
    }
  }

  return true;
}

bool Checker::areEqualCombSat(const std::vector<const GNet*> &nets,
                              const GateConnect *connectTo,
                              const GateBinding &ibind,
                              const GateBinding &obind) const {
  Encoder encoder;
  encoder.setConnectTo(connectTo);

  // Equate the inputs.
  for (const auto &[lhsGateLink, rhsGateLink] : ibind) {
    const auto x = encoder.var(lhsGateLink.source, 0);
    const auto y = encoder.var(rhsGateLink.source, 0);

    encoder.encodeBuf(y, x, true);
  }

  // Encode the nets.
  for (const auto *net : nets) {
    encoder.encode(*net, 0);
  }

  // Compare the outputs.
  Context::Clause existsDiff;
  for (const auto &[lhsGateLink, rhsGateLink] : obind) {
    const auto y  = encoder.newVar();
    const auto x1 = encoder.var(lhsGateLink.source, 0);
    const auto x2 = encoder.var(rhsGateLink.source, 0);

    encoder.encodeXor(y, x1, x2, true, true, true);
    existsDiff.push(Context::lit(y, true));
  }

  // (lOut[1] != rOut[1]) || ... || (lOut[m] != rOut[m]).
  encoder.encode(existsDiff);

  const auto verdict = !encoder.solve();

  if (!verdict) {
    error(encoder.context(), ibind, obind);
  }

  return verdict;
}

void Checker::error(Context &context,
                    const GateBinding &ibind,
                    const GateBinding &obind) const {
  bool comma;
  context.dump("miter.cnf");

  comma = false;
  std::cout << "Inputs: ";
  for (const auto &[lhsGateLink, rhsGateLink] : ibind) {
    if (comma) std::cout << ", ";
    comma = true;

    std::cout << context.value(context.var(lhsGateLink.source, 0)) << "|";
    std::cout << context.value(context.var(rhsGateLink.source, 0));
  }
  std::cout << std::endl;

  comma = false;
  std::cout << "Outputs: ";
  for (const auto &[lhsGateLink, rhsGateLink] : obind) {
    if (comma) std::cout << ", ";
    comma = true;

    std::cout << context.value(context.var(lhsGateLink.source, 0)) << "|";
    std::cout << context.value(context.var(rhsGateLink.source, 0));
  }
  std::cout << std::endl;
}

} // namespace eda::gate::debugger
