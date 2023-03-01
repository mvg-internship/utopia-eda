#include <iostream>
#include "passes/techmap/libparse.h"
#include <kernel/yosys.h>
#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "base/model/signal.h"
#include "gate/simulator/simulator.h"
#include <map>
#include<string>
using namespace eda::gate::model;
int root;
void print_modules(const int ind, std::ostream &out){
    for (const auto &it1 : Yosys::RTLIL::IdString::global_id_index_){
        if (it1.second == ind){
            out << it1.first << " - module of index: " << it1.second << "\n";
        }
    }
}

void print_wires(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Wire*> &wires, std::ostream &out, eda::gate::model::GNet &net,std::map<unsigned, Gate::Id> &inputs,std::map<unsigned, Gate::Id> &outputs){
    out << "Wires:" << "\n";
    for (auto it1=wires.begin(); it1 != wires.end(); ++it1){
        unsigned index;
        for(const auto &it2 : Yosys::RTLIL::IdString::global_id_index_){

            if (it2.second == it1->first.index_){
                out << "  " << it2.first << " - wire of index: " << it2.second << "\n";
                index=it2.second;
            }
        }
        if (it1->second->width>1){
            out<< "    type: bus with width " << it1->second->width << "\n";
        }
        else{
            out << "    type: wire" << "\n";
        }
        out << "    start_offset: " << it1->second->start_offset << "\n";
        out << "    port_id: " << it1->second->port_id << "\n";
        out << "    port_input: " << it1->second->port_input << "\n";
        if (it1->second->port_input==1){
            Gate::Id inputId = net.addIn();
            inputs.emplace(index, inputId);
        }
        out << "    port_output: " << it1->second->port_output << "\n";
        if (it1->second->port_output==1){
            Gate::Id outputId;
            outputs.emplace(index,outputId);
        }
        out << "    upto: " << it1->second->upto << "\n";
        out << "\n";
    }
}
GateSymbol function(size_t type){
    GateSymbol func;
    if (type==355){
        func=GateSymbol::NOT;
    }
    if (type==356){
        func=GateSymbol::AND;
    }
    if (type==358){
        func=GateSymbol::OR;
    }
    if (type==360){
        func=GateSymbol::XOR;
    }
    return func;
}
Gate::Id buildNet(int root,eda::gate::model::GNet &net,std::map<int,GateSymbol> typeFunc,std::map<int, std::pair<int, int>> &cell,std::map<unsigned, Gate::Id> &inputs){

    bool flag1=0,flag2=0;
    if(inputs.find(cell.find(root)->second.first)!=inputs.end()){
        flag1=1;
    }
    if(inputs.find(cell.find(root)->second.second)!=inputs.end()){
        flag2=1;
    }
    if (flag1==1&&flag2==1){
        Gate::SignalList inputs1;
        if (cell.find(root)->second.first!=cell.find(root)->second.second){
            inputs1.push_back(Gate::Signal::always(inputs.find(cell.find(root)->second.first)->second));
            inputs1.push_back(Gate::Signal::always(inputs.find(cell.find(root)->second.second)->second));
        }
        else{
            inputs1.push_back(Gate::Signal::always(inputs.find(cell.find(root)->second.first)->second));
        }
        return net.addGate(typeFunc.find(root)->second,inputs1);
    }
    if (flag1==0&&flag2==0){
        Gate::SignalList inputs1;
        if (cell.find(root)->second.first!=cell.find(root)->second.second){
            inputs1.push_back(Gate::Signal::always(buildNet(cell.find(root)->second.first,net,typeFunc,cell,inputs)));
            inputs1.push_back(Gate::Signal::always(buildNet(cell.find(root)->second.second,net,typeFunc,cell,inputs)));
        }
        else{
            inputs1.push_back(Gate::Signal::always(buildNet(cell.find(root)->second.first,net,typeFunc,cell,inputs)));
        }
        return net.addGate(typeFunc.find(root)->second,inputs1);
    }
    if (flag1==0&&flag2==1){
        Gate::SignalList inputs1;
        inputs1.push_back(Gate::Signal::always(buildNet(cell.find(root)->second.first,net,typeFunc,cell,inputs)));
        inputs1.push_back(Gate::Signal::always(inputs.find(cell.find(root)->second.second)->second));
        return net.addGate(typeFunc.find(root)->second,inputs1);
    }
    if (flag1==1&&flag2==0){
        Gate::SignalList inputs1;
        inputs1.push_back(Gate::Signal::always(inputs.find(cell.find(root)->second.first)->second));
        inputs1.push_back(Gate::Signal::always(buildNet(cell.find(root)->second.second,net,typeFunc,cell,inputs)));
        return net.addGate(typeFunc.find(root)->second,inputs1);
    }
}

void print_cells(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Cell*> &cells, std::ostream &out, eda::gate::model::GNet &net, std::map<int,GateSymbol> &typeFunc,std::map<int, std::pair<int, int>> &cell){
    out << "Cells:" << "\n";
    int p=0;
    for (auto it1=cells.begin(); it1 != cells.end(); ++it1){
        for (const auto &it2 : Yosys::RTLIL::IdString::global_id_index_){
            if (it2.second == it1->first.index_){
                out << "  " << it2.first << " cell of index " << it2.second << " type " << it1->second->type.index_ << "\n";
            }
        }
        size_t i=0;
        bool flag=0;
        GateSymbol f;
        int a,b,c;
        for (auto it3 = it1->second->connections_.begin(); it3 != it1->second->connections_.end(); ++it3){
            i++;
            p++;
            if(p==1){
                root=it3->second.as_wire()->name.index_;
            }
            if (i==1){
                a=it3->second.as_wire()->name.index_;
                f=function(it1->second->type.index_);
                typeFunc.emplace(a,f);
                if (f==GateSymbol::NOT){
                    flag=1;
                }
            }
            if (i==2){
                if (!flag){
                    b=it3->second.as_wire()->name.index_;
                    flag=0;
                }
                else{
                    b=it3->second.as_wire()->name.index_;
                    c=b;
                    cell.emplace(a,std::make_pair(b,c));
                }
            }
            if (i==3){
                c=it3->second.as_wire()->name.index_;
                cell.emplace(a,std::make_pair(b,c));
            }
            out << "    Connections: " << it3->first.index_ << "\n";
            out << "      name: " << it3->second.as_wire()->name.index_ << "\n";
        }
    }
}
void print_connections(const std::vector<std::pair<Yosys::RTLIL::SigSpec, Yosys::RTLIL::SigSpec> > &connections, std::ostream &out,eda::gate::model::GNet &net,std::map<int,GateSymbol> typeFunc,std::map<int, std::pair<int, int>> &cell,std::map<unsigned, Gate::Id> &inputs,std::map<unsigned, Gate::Id> &outputs){
    out << "Connections count: " << connections.size() << "\n";
    for (auto it1 = connections.begin(); it1 != connections.end(); ++it1){
        out<<"    Wire " << it1->first.as_wire()->name.index_ << " connects with " << it1->second.as_wire()->name.index_ << "\n";
        if(inputs.find(it1->second.as_wire()->name.index_)==inputs.end()){
            root=it1->second.as_wire()->name.index_;
            auto output=buildNet(root,net,typeFunc,cell,inputs);
            outputs.find(it1->first.as_wire()->name.index_)->second=net.addOut(output);
            std::cout << net;
        }
        else{
            Gate::SignalList inputs1;
            inputs1.push_back(Gate::Signal::always(inputs.find(it1->second.as_wire()->name.index_)->second));
            auto output=net.addGate(GateSymbol::NOP,inputs1);
            outputs.find(it1->first.as_wire()->name.index_)->second=net.addOut(output);
            std::cout << net;
        }
    }
}

void print_memory(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Memory*> &memories, std::ostream &out){
    out << "Memory count: " << memories.size() << "\n";
    for (auto it1 = memories.begin(); it1 != memories.end(); ++it1){
        out << "  memory " << it1->second << "\n";
        out << "  name: " << it1->second->name.index_ << "\n";
        out << "  width: " << it1->second->width << "\n";
        out << "  start_offset: " << it1->second->start_offset << "\n";
        out << "  size: " << it1->second->size << "\n";
        out << "\n";
    }
}
void print_actions(const std::vector< Yosys::RTLIL::SigSig > &actions, std::ostream &out){
    for (auto &it : actions){
        out << "    " << (*it.first.as_chunk().wire).name.index_ << "\n";
    }
}
void print_syncs(const std::vector<Yosys::RTLIL::SyncRule *> &syncs, std::ostream &out){
    out << "  syncs\n";
    for (auto it1=syncs.begin(); it1 != syncs.end(); ++it1){
        out << "    type " << (*it1)->type << "\n";
        out << "  signal\n";
        out << "    size " << (*it1)->signal.size() << "\n";
        out << "    as_wire index " << (*it1)->signal.as_wire()->name.index_ << "\n";
        out << "  actions\n";
        print_actions((*it1)->actions,out);
    }
}


void print_processes(const Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Process*> &processes, std::ostream &out){
    out << "Processes count: " << processes.size() << "\n";
    for (auto it1 = processes.begin(); it1 != processes.end(); ++it1){
        out << "  name " << it1->second->name.index_ << "\n";
        print_syncs(it1->second->syncs, out);
        out << "\n";
    }
}

void print_ports(const std::vector<Yosys::RTLIL::IdString> &ports, std::ostream &out){
    out << "Ports count: " << ports.size() << "\n";
    for (auto &it1 : ports){
        out << "  port " << it1.index_ << "\n";
    }
}

void print_params(const std::pair<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*> &m, std::ostream &out){
    eda::gate::model::GNet net(0);
    std::map<unsigned, Gate::Id> outputs;
    std::map<unsigned, Gate::Id> inputs;
    std::map<int, std::pair<int, int>> cell;
    std::map<int,GateSymbol> typeFunc;
    print_modules(m.first.index_, out);
    out << "\n";
    print_wires(m.second->wires_, out,net,inputs,outputs);
    out << "\n";
    print_cells(m.second->cells_, out,net,typeFunc,cell);
    out << "\n" ;
    print_connections(m.second->connections_, out,net,typeFunc,cell,inputs,outputs);
    out << "\n";
    print_memory(m.second->memories, out);
    out << "\n";
    print_processes(m.second->processes, out);
    out << "\n";
    print_ports(m.second->ports, out);
    out << "\n";

    out << "Refcount wires count: " << m.second->refcount_wires_ << "\n";
    out << "Refcount cells count: " << m.second->refcount_cells_ << "\n";
    out << "Monitors count: " << m.second->monitors.size() << "\n";
    out << "Avail_parameters count: " << m.second->avail_parameters.size() << "\n";
    out << "\n";
}

void print_parsed(const Yosys::RTLIL::Design &des, std::ostream &out){
    for (auto &m: des.modules_){
        print_params(m, out);
    }
}
int main(int argc, char* argv[]){
    std::ostream& out = std::cout;
    Yosys::yosys_setup();
    for (size_t o=1;o<argc;++o){
        Yosys::RTLIL::Design design;
        Yosys::run_frontend(argv[o], "liberty", &design, nullptr);
        print_parsed(design, out);
    }
    Yosys::yosys_shutdown();
}



