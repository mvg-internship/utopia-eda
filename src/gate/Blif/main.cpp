#include <iostream>
#include <frontends/blif/blifparse.h>
#include "kernel/yosys.h"
#include "kernel/register.h"
#include <vector>
#include <cstring>

void parse(std::string filename) {
    std::ifstream f(filename);
    Yosys::RTLIL::Design des;
    bool sop_mode = false;
    bool wideports = false;

    Yosys::parse_blif(&des, f, "", true, sop_mode, wideports);
    std::cout << "hashidx_  " << des.hashidx_ << std::endl;
    std::cout << "refcount_modules_  " << des.refcount_modules_ << std::endl;
    std::cout << "selected_active_module " << des.selected_active_module << std::endl;
    std::cout << "scratchpad:" << std::endl;
    for (auto it = des.scratchpad.begin(); it != des.scratchpad.end(); ++it)
    {
        std::cout << it->first << " " << it->second << std::endl;
    }
    std::cout << "Monitors:" << std::endl;

    for (auto i : des.monitors)
        std::cout << i->hashidx_ << ' ';
    std::cout << "Modules:" << std::endl;

    for (auto it = des.modules_.begin(); it != des.modules_.end(); ++it)
    {
        std::cout << "global_id_storage_: " << std::endl;
        for (char* i : it->first.global_id_storage_)
            std::cout << i << " ";
        std::cout << "global_id_index_: " << std::endl;
        for (auto it1 = it->first.global_id_index_.begin(); it1 != it->first.global_id_index_.end(); ++it1)
        {
            std::cout << it1->first << " " << it1->second << std::endl;
        }
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
        std::cout << "memories:" << std::endl;
        for (auto it2 = it->second->memories.begin(); it2 != it->second->memories.end(); ++it2) {
            std::cout << "width: " << it2->second->width << std::endl;
            std::cout << "start_offset: " << it2->second->start_offset << std::endl;
            std::cout << "size: " << it2->second->size << std::endl;
        }

    }
    std::cout << "selection stack:" << std::endl;
    for (auto i : des.selection_stack) {
        std::cout << i.full_selection << '\n';
        std::cout << "selected_modules & selected_members\n";
        std::cout << "Look up at the field IdString\n";
    }
}

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++){
        parse(argv[i]);
    }
    return 0;
}

