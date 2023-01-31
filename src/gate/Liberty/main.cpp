#include "passes/techmap/libparse.h"
#include <kernel/yosys.h>
void print_modules(const int ind, Yosys::hashlib::dict<char*, int, Yosys::hashlib::hash_cstr_ops> global_id_index_, std::ostream &out){
    for (auto &it1 : global_id_index_){
        if (it1.second == ind){
            out << it1.first << " - module of index: " << it1.second << "\n";
        }
    }
}

void print_wires(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Wire*> wires, std::ostream &out){
    out << "Wires:" << "\n";
    for (auto it1=wires.begin(); it1 != wires.end(); ++it1){
        for(auto &it2 : Yosys::RTLIL::IdString::global_id_index_){
            if (it2.second == it1->first.index_){
                out << "  " << it2.first << " - wire of index: " << it2.second << "\n";
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
        out << "    port_output: " << it1->second->port_output << "\n";
        out << "    upto: " << it1->second->upto << "\n";
        out << "\n";
    }
}

void print_cells(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Cell*> cells, std::ostream &out){
    out << "Cells:" << "\n";
    for (auto it1=cells.begin(); it1 != cells.end(); ++it1){
        for (auto &it2 : Yosys::RTLIL::IdString::global_id_index_){
            if (it2.second == it1->first.index_){
                out << "  " << it2.first << " cell of index " << it2.second << " type " << it1->second->type.index_ << "\n";
            }
        }
        for (auto it3 = it1->second->connections_.begin(); it3 != it1->second->connections_.end(); ++it3){
            out << "    Connections: " << it3->first.index_ << "\n";
            out << "      name: " << it3->second.as_wire()->name.index_ << "\n";
        }
    }
}

void print_connections(std::vector<std::pair<Yosys::RTLIL::SigSpec, Yosys::RTLIL::SigSpec> > connections, std::ostream &out){
    out << "Connections count: " << connections.size() << "\n";
    for (auto it1 = connections.begin(); it1 != connections.end(); ++it1){
        out<<"    Wire " << it1->first.as_wire()->name.index_ << " connects with " << it1->second.as_wire()->name.index_ << "\n";
    }
}

void print_memory(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Memory*> memories, std::ostream &out){
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

void print_syncs(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Process*>::iterator it, std::ostream &out){
    out << "  syncs\n";
    for (auto it1=it->second->syncs.begin(); it1 != it->second->syncs.end(); ++it1){
        out << "    type " << (*it1)->type << "\n";
        out << "  signal\n";
        out << "    size " << (*it1)->signal.size() << "\n";
        out << "    as_wire index " << (*it1)->signal.as_wire()->name.index_ << "\n";
        out << "  actions\n";
        for (auto &it2 : (*it1)->actions){
            out << "    " << (*it2.first.as_chunk().wire).name.index_ << "\n";
        }
    }
}

void print_processes(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Process*> processes, std::ostream &out){
    out << "Processes count: " << processes.size() << "\n";
    for (auto it1 = processes.begin(); it1 != processes.end(); ++it1){
        out << "  name " << it1->second->name.index_ << "\n";
        out << "  root_case\n"; //Does not print (ToDo)
        //for (auto &it3 : it1->second->root_case.actions){
        //    out << "    " << (it3.first.as_wire())->name.index_ << "\n";
        //}
        print_syncs(it1, out);
        out << "\n";
    }
}

void print_ports(std::vector<Yosys::RTLIL::IdString> ports, std::ostream &out){
    out << "Ports count: " << ports.size() << "\n";
    for (auto &it1 : ports){
        out << "  port " << it1.index_ << "\n";
    }
}

void print_params(const std::pair<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*> &m, std::ostream &out){
    print_modules(m.first.index_, Yosys::RTLIL::IdString::global_id_index_, out);
    out << "\n";
    print_wires(m.second->wires_, out);
    out << "\n";
    print_cells(m.second->cells_, out);
    out << "\n" ;
    print_connections(m.second->connections_, out);
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

void print_parsed(Yosys::RTLIL::Design &des, std::ostream &out){
    for (const auto &m: des.modules_){
        print_params(m, out);
    }
}
int main(int argc, char* argv[]){
    Yosys::log_streams.push_back(&std::cout);
        Yosys::log_error_stderr = true;
        std::ostream& out = std::cout;
        Yosys::yosys_setup();
        for (size_t o=1;o<argc;++o){
            Yosys::RTLIL::Design design;
            Yosys::run_frontend(argv[o], "liberty", &design, nullptr);
            print_parsed(design, out);
        }
        Yosys::yosys_shutdown();
}


