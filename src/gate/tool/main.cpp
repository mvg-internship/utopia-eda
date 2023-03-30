#include <iostream>
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "base/model/signal.h"
#include "gate/simulator/simulator.h"
#include <vector>
#include <cmath>
using namespace eda::gate::model;
void print_table(eda::gate::model::GNet &net, Gate::SignalList &inputs,Gate::Id &outputId){
    static eda::gate::simulator::Simulator simulator;
    GNet::LinkList in;
    GNet::LinkList out{Gate::Link(outputId)};

    for (auto input : inputs) {
        in.push_back(Gate::Link(input.node()));
    }

    auto compiled = simulator.compile(net, in, out);

    std::uint64_t OUT;
    for (std::uint64_t i = 0; i < std::pow(2,net.nSourceLinks()); i++) {
        compiled.simulate(OUT, i);
        std::cout << std::hex << i << " " << OUT << "\n";
    }
}
int main(){
    std::cout<<"OR op\n";
    eda::gate::model::GNet net(0);
    Gate::SignalList inputs;
    Gate::Id outputId;
    for (unsigned i = 0; i < 2; i++) {
        const Gate::Id inputId = net.addIn();
        inputs.push_back(Gate::Signal::always(inputId));
    }
    auto gateId = net.addGate(GateSymbol::OR, inputs);
    outputId = net.addOut(gateId);
   // auto gate = net.addGate(GateSymbol::NOT, gateId);
 //   outputId = net.addOut(gate);
    net.sortTopologically();
    std::cout<<net;
    print_table(net,inputs,outputId);
    std::cout<<"AND op\n";
    eda::gate::model::GNet net1(0);
    Gate::SignalList inputs1;
    Gate::Id outputId1;
    for (unsigned i = 0; i < 2; i++) {
        const Gate::Id inputId1 = net1.addIn();
        inputs1.push_back(Gate::Signal::always(inputId1));
    }
    auto gateId1 = net1.addGate(GateSymbol::NOT, inputs1);
    outputId1 = net1.addOut(gateId1);
    net1.sortTopologically();
    print_table(net1,inputs1,outputId1);
}
