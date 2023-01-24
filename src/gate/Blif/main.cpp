#include <iostream>
#include <frontends/blif/blifparse.h>
#include "kernel/yosys.h"
#include "kernel/register.h"

void parse(char *filename, Yosys::RTLIL::Design &des) {
    std::ifstream f(filename);
    bool sop_mode = false;
    bool wideports = false;

    Yosys::parse_blif(&des, f, "", true, sop_mode, wideports);
}

void show_scratchpads(Yosys::RTLIL::Design &des){
    std::cout << "scratchpad:" << std::endl;
    for (auto it = des.scratchpad.begin(); it != des.scratchpad.end(); ++it) {
        std::cout << it->first << " " << it->second << std::endl;
    }
}

void show_monitors(Yosys::RTLIL::Design &des){
    std::cout << "Monitors:" << std::endl;
    for (auto i : des.monitors)
        std::cout << i->hashidx_ << ' ';
}

void show_storage(Yosys::RTLIL::Design &des, Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator &it){
    std::cout << "global_id_storage_: " << std::endl;
    for (char* i : it->first.global_id_storage_)
        std::cout << i << " ";
}

void show_idx(Yosys::RTLIL::Design &des, Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator &it){
    std::cout << "global_id_index_: " << std::endl;
    for (auto it1 = it->first.global_id_index_.begin(); it1 != it->first.global_id_index_.end(); ++it1) {
        std::cout << it1->first << " " << it1->second << std::endl;
    }
}

void show_wires(Yosys::RTLIL::Design &des, Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator &it){
    std::cout << "refcount_wires_: " << it->second->refcount_wires_ << std::endl;
    std::cout << "refcount_cells_: " << it->second->refcount_cells_ << std::endl;
    for (auto it2 = it->second->wires_.begin(); it2 != it->second->wires_.end(); ++it2) {
        std::cout << "width: " << it2->second->width << std::endl;
        std::cout << "start_offset: " << it2->second->start_offset << std::endl;
        std::cout << "port_id: " << it2->second->port_id << std::endl;
        std::cout << "port_input: " << it2->second->port_input << std::endl;
        std::cout << "port_output: " << it2->second->port_output << std::endl;
        std::cout << "upto: " << it2->second->upto << std::endl;
    }
}

void show_memories(Yosys::RTLIL::Design &des, Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator &it){
    std::cout << "memories:" << std::endl;
    for (auto it2 = it->second->memories.begin(); it2 != it->second->memories.end(); ++it2) {
        std::cout << "width: " << it2->second->width << std::endl;
        std::cout << "start_offset: " << it2->second->start_offset << std::endl;
        std::cout << "size: " << it2->second->size << std::endl;
    }
}

void show_selection_stack(Yosys::RTLIL::Design &des){
    std::cout << "selection stack:" << std::endl;
    for (auto i : des.selection_stack) {
        std::cout << i.full_selection << '\n';
        std::cout << "selected_modules & selected_members\n";
        std::cout << "Look up at the field IdString\n";
    }
}

void show_info(Yosys::RTLIL::Design &des){
    std::cout << "hashidx_  " << des.hashidx_ << std::endl;
    std::cout << "refcount_modules_  " << des.refcount_modules_ << std::endl;
    std::cout << "selected_active_module " << des.selected_active_module << std::endl;
    show_scratchpads(des);
    show_monitors(des);
    std::cout << "Modules:" << std::endl;
    for (auto it = des.modules_.begin(); it != des.modules_.end(); ++it) {
        show_storage(des, it);
        show_idx(des, it);
        show_wires(des, it);
        show_memories(des, it);
    }
    show_selection_stack(des);
}

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++) {
        std::cout << "Filename: " << argv[i] << std::endl;
        Yosys::RTLIL::Design des;
        parse(argv[i], des);
        show_info(des);
        std::cout << std::endl;
    }
    return 0;
}

