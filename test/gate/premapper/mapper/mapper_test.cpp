#include "mapper_test.h"

// gate(x1, ..., xN).
std::shared_ptr<GNet> makeSingleGateNet(GateSymbol gate,
                                        const unsigned N) {
  std::shared_ptr<GNet> net = std::make_shared<GNet>();

  Gate::SignalList inputs;

  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->addIn();
    inputs.push_back(Gate::Signal::always(inputId));
  }

  auto gateId = net->addGate(gate, inputs);
  net->addOut(gateId);

  net->sortTopologically();
  return net;
}

// gate(~x1, ..., ~xN).
std::shared_ptr<GNet> makeSingleGateNetn(GateSymbol gate,
                                         const unsigned N) {
  std::shared_ptr<GNet> net = std::make_shared<GNet>();

  Gate::SignalList inputs;

  Gate::SignalList andInputs;
  for (unsigned i = 0; i < N; i++) {
    const Gate::Id inputId = net->addIn();
    inputs.push_back(Gate::Signal::always(inputId));

    const Gate::Id notGateId = net->addNot(inputId);
    andInputs.push_back(Gate::Signal::always(notGateId));
  }

  auto gateId = net->addGate(gate, andInputs);
  net->addOut(gateId);

  net->sortTopologically();
  return net;
}

// gate(x1, ~x2).
std::shared_ptr<GNet> makeSingleGateNetOppositeInputs(GateSymbol gate) {
  std::shared_ptr<GNet> net = std::make_shared<GNet>();

  Gate::SignalList inputs;
  Gate::SignalList andInputs;

  const Gate::Id inputId = net->addIn();
  inputs.push_back(Gate::Signal::always(inputId));

  const Gate::Id notGateId = net->addNot(inputId);
  andInputs.push_back(Gate::Signal::always(notGateId));

  andInputs.push_back(Gate::Signal::always(inputId));

  auto gateId = net->addGate(gate, andInputs);
  net->addOut(gateId);

  net->sortTopologically();
  return net;
}

std::shared_ptr<GNet> premap(std::shared_ptr<GNet> net,
                             GateIdMap &gmap,
                             PreBasis basis) {
  std::shared_ptr<GNet> premapped = eda::gate::premapper::getPreMapper(basis).map(*net, gmap);
  premapped->sortTopologically();
  return premapped;
}

bool checkEquivalence(const std::shared_ptr<GNet> net,
                      const std::shared_ptr<GNet> premapped,
                      GateIdMap &gmap) {
  eda::gate::debugger::Checker checker;
  return checker.areEqual(*net, *premapped, gmap);
}
