#include "frontends/verilog/verilog_frontend.h"
#include "frontends/verilog/preproc.h"
#include "kernel/yosys.h"
#include "kernel/register.h"
#include "libs/sha1/sha1.h"
#include "frontends/ast/ast.h"
#include <stdarg.h>
#include <iostream>
#include <istream>
#include <cstring>

YOSYS_NAMESPACE_BEGIN
using namespace VERILOG_FRONTEND;


static std::vector<std::string> verilog_defaults;
static std::list<std::vector<std::string>> verilog_defaults_stack;

static void add_package_types1(dict<std::string, AST::AstNode *> &user_types, std::vector<AST::AstNode *> &package_list){
	for (const auto &pkg : package_list) {
		for (const auto &node: pkg->children) {
            if (node->type == Yosys::AST::AST_TYPEDEF) {
				std::string s = pkg->str + "::" + node->str.substr(1);
				user_types[s] = node;
			}
		}
	}
}

YOSYS_NAMESPACE_END

void parse(std::ifstream &f, std::string filename, Yosys::RTLIL::Design *design, std::vector<std::string> args){

    bool flag_dump_ast1 = false;
    bool flag_dump_ast2 = false;
    bool flag_no_dump_ptr = false;
    bool flag_dump_vlog1 = false;
    bool flag_dump_vlog2 = false;
    bool flag_dump_rtlil = false;
    bool flag_nolatches = false;
    bool flag_nomeminit = false;
    bool flag_nomem2reg = false;
    bool flag_mem2reg = false;
    bool flag_ppdump = false;
    bool flag_nopp = false;
    bool flag_nodpi = false;
    bool flag_noopt = false;
    bool flag_icells = false;
    bool flag_pwires = false;
    bool flag_nooverwrite = false;
    bool flag_overwrite = false;
    bool flag_defer = false;
    bool flag_noblackbox = false;
    bool flag_nowb = false;
    bool flag_nosynthesis = false;
    Yosys::define_map_t defines_map;

    std::list<std::string> include_dirs;
    std::list<std::string> attributes;

    frontend_verilog_yydebug = false;
    Yosys::VERILOG_FRONTEND::sv_mode = false;
    Yosys::VERILOG_FRONTEND::formal_mode = false;
    Yosys::VERILOG_FRONTEND::norestrict_mode = false;
    Yosys::VERILOG_FRONTEND::assume_asserts_mode = false;
    Yosys::VERILOG_FRONTEND::lib_mode = false;
    Yosys::VERILOG_FRONTEND::specify_mode = false;
    Yosys::VERILOG_FRONTEND::default_nettype_wire = true;

    args.insert(args.begin()+1, Yosys::verilog_defaults.begin(), Yosys::verilog_defaults.end());

    size_t argidx;
    for (argidx = 1; argidx < args.size(); argidx++) {
        std::string arg = args[argidx];
        if (arg == "-sv") {
            Yosys::VERILOG_FRONTEND::sv_mode = true;
            continue;
        }
        if (arg == "-formal") {
            Yosys::VERILOG_FRONTEND::formal_mode = true;
            continue;
        }
        if (arg == "-nosynthesis") {
            flag_nosynthesis = true;
            continue;
        }
        if (arg == "-noassert") {
            Yosys::VERILOG_FRONTEND::noassert_mode = true;
            continue;
        }
        if (arg == "-noassume") {
            Yosys::VERILOG_FRONTEND::noassume_mode = true;
            continue;
        }
        if (arg == "-norestrict") {
            Yosys::VERILOG_FRONTEND::norestrict_mode = true;
            continue;
        }
        if (arg == "-assume-asserts") {
            Yosys::VERILOG_FRONTEND::assume_asserts_mode = true;
            continue;
        }
        if (arg == "-assert-assumes") {
            Yosys::VERILOG_FRONTEND::assert_assumes_mode = true;
            continue;
        }
        if (arg == "-debug") {
            flag_dump_ast1 = true;
            flag_dump_ast2 = true;
            flag_dump_vlog1 = true;
            flag_dump_vlog2 = true;
            frontend_verilog_yydebug = true;
            continue;
        }
        if (arg == "-dump_ast1") {
            flag_dump_ast1 = true;
            continue;
        }
        if (arg == "-dump_ast2") {
            flag_dump_ast2 = true;
            continue;
        }
        if (arg == "-no_dump_ptr") {
            flag_no_dump_ptr = true;
            continue;
        }
        if (arg == "-dump_vlog1") {
            flag_dump_vlog1 = true;
            continue;
        }
        if (arg == "-dump_vlog2") {
            flag_dump_vlog2 = true;
            continue;
        }
        if (arg == "-dump_rtlil") {
            flag_dump_rtlil = true;
            continue;
        }
        if (arg == "-yydebug") {
            frontend_verilog_yydebug = true;
            continue;
        }
        if (arg == "-nolatches") {
            flag_nolatches = true;
            continue;
        }
        if (arg == "-nomeminit") {
            flag_nomeminit = true;
            continue;
        }
        if (arg == "-nomem2reg") {
            flag_nomem2reg = true;
            continue;
        }
        if (arg == "-mem2reg") {
            flag_mem2reg = true;
            continue;
        }
        if (arg == "-ppdump") {
            flag_ppdump = true;
            continue;
        }
        if (arg == "-nopp") {
            flag_nopp = true;
            continue;
        }
        if (arg == "-nodpi") {
            flag_nodpi = true;
            continue;
        }
        if (arg == "-noblackbox") {
            flag_noblackbox = true;
            continue;
        }
        if (arg == "-lib") {
            Yosys::VERILOG_FRONTEND::lib_mode = true;
            defines_map.add("BLACKBOX", "");
            continue;
        }
        if (arg == "-nowb") {
            flag_nowb = true;
            continue;
        }
        if (arg == "-specify") {
            Yosys::VERILOG_FRONTEND::specify_mode = true;
            continue;
        }
        if (arg == "-noopt") {
            flag_noopt = true;
            continue;
        }
        if (arg == "-icells") {
            flag_icells = true;
            continue;
        }
        if (arg == "-pwires") {
            flag_pwires = true;
            continue;
        }
        if (arg == "-ignore_redef" || arg == "-nooverwrite") {
            flag_nooverwrite = true;
            flag_overwrite = false;
            continue;
        }
        if (arg == "-overwrite") {
            flag_nooverwrite = false;
            flag_overwrite = true;
            continue;
        }
        if (arg == "-defer") {
            flag_defer = true;
            continue;
        }
        if (arg == "-noautowire") {
            Yosys::VERILOG_FRONTEND::default_nettype_wire = false;
            continue;
        }
        if (arg == "-setattr" && argidx+1 < args.size()) {
            attributes.push_back(Yosys::RTLIL::escape_id(args[++argidx]));
            continue;
        }
        if (arg == "-D" && argidx+1 < args.size()) {
            std::string name = args[++argidx], value;
            size_t equal = name.find('=');
            if (equal != std::string::npos) {
                value = name.substr(equal+1);
                name = name.substr(0, equal);
            }
            defines_map.add(name, value);
            continue;
        }
        if (arg.compare(0, 2, "-D") == 0) {
            size_t equal = arg.find('=', 2);
            std::string name = arg.substr(2, equal-2);
            std::string value;
            if (equal != std::string::npos)
                value = arg.substr(equal+1);
            defines_map.add(name, value);
            continue;
        }
        if (arg == "-I" && argidx+1 < args.size()) {
            include_dirs.push_back(args[++argidx]);
            continue;
        }
        if (arg.compare(0, 2, "-I") == 0) {
            include_dirs.push_back(arg.substr(2));
            continue;
        }
        break;
    }

    if (Yosys::VERILOG_FRONTEND::formal_mode || !flag_nosynthesis)
        defines_map.add(Yosys::VERILOG_FRONTEND::formal_mode ? "FORMAL" : "SYNTHESIS", "1");

    //Yosys::Pass check;
    //check.extra_args(args, argidx, design, 1);


    Yosys::AST::current_filename = filename;
    Yosys::AST::set_line_num = &frontend_verilog_yyset_lineno;
    Yosys::AST::get_line_num = &frontend_verilog_yyget_lineno;

    Yosys::VERILOG_FRONTEND::current_ast = new Yosys::AST::AstNode(Yosys::AST::AST_DESIGN);

    Yosys::VERILOG_FRONTEND::lexin = &f;
    std::string code_after_preproc;

    if (!flag_nopp) {
        code_after_preproc = frontend_verilog_preproc(f, filename, defines_map, *design->verilog_defines, include_dirs);
        Yosys::VERILOG_FRONTEND::lexin = new std::istringstream(code_after_preproc);
    }

    Yosys::add_package_types1(Yosys::VERILOG_FRONTEND::pkg_user_types, design->verilog_packages);

    Yosys::VERILOG_FRONTEND::UserTypeMap global_types_map;
    for (auto def : design->verilog_globals) {
        if (def->type == Yosys::AST::AST_TYPEDEF) {
            global_types_map[def->str] = def;
        }
    }

    Yosys::VERILOG_FRONTEND::user_type_stack.push_back(std::move(global_types_map));
    Yosys::VERILOG_FRONTEND::user_type_stack.push_back(Yosys::VERILOG_FRONTEND::UserTypeMap());

    frontend_verilog_yyset_lineno(1);
    frontend_verilog_yyrestart(NULL);
    frontend_verilog_yyparse();
    frontend_verilog_yylex_destroy();

    for (auto &child : Yosys::VERILOG_FRONTEND::current_ast->children) {
        if (child->type == Yosys::AST::AST_MODULE)
            for (auto &attr : attributes)
                if (child->attributes.count(attr) == 0)
                    child->attributes[attr] = Yosys::AST::AstNode::mkconst_int(1, false);
    }


    Yosys::AST::process(design, Yosys::VERILOG_FRONTEND::current_ast, flag_dump_ast1, flag_dump_ast2, flag_no_dump_ptr, flag_dump_vlog1, flag_dump_vlog2, flag_dump_rtlil, flag_nolatches,
            flag_nomeminit, flag_nomem2reg, flag_mem2reg, flag_noblackbox, Yosys::VERILOG_FRONTEND::lib_mode, flag_nowb, flag_noopt, flag_icells, flag_pwires, flag_nooverwrite, flag_overwrite, flag_defer, Yosys::VERILOG_FRONTEND::default_nettype_wire);


    if (!flag_nopp)
        delete Yosys::VERILOG_FRONTEND::lexin;

    // only the previous and new global type maps remain
    Yosys::VERILOG_FRONTEND::user_type_stack.clear();

    delete Yosys::VERILOG_FRONTEND::current_ast;
    Yosys::VERILOG_FRONTEND::current_ast = NULL;
}

void print_modules(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator it, Yosys::hashlib::dict<char*, int, Yosys::hashlib::hash_cstr_ops> global_id_index_, std::ostream &out){
    for (auto &it1 : global_id_index_){
        if (it1.second == it->first.index_){
            out << it1.first << " - module of index: " << it1.second << "\n";
        }
    }
}

void print_wires(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator it, std::ostream &out){
    out << "Wires:" << "\n";
    for (auto it1=it->second->wires_.begin(); it1 != it->second->wires_.end(); ++it1){
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

void print_cells(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator it, std::ostream &out){
    out << "Cells:" << "\n";
    for (auto it1=it->second->cells_.begin(); it1 != it->second->cells_.end(); ++it1){
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

void print_connections(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator it, std::ostream &out){
    out << "Connections count: " << it->second->connections_.size() << "\n";
    for (auto it1 = it->second->connections_.begin(); it1 != it->second->connections_.end(); ++it1){
        out<<"  Wire " << it1->first.as_wire()->name.index_ << " connects with " << it1->second.as_wire()->name.index_ << "\n";
    }
}

void print_memory(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator it, std::ostream &out){
    out << "Memory count: " << it->second->memories.size() << "\n";
    for (auto it1 = it->second->memories.begin(); it1 != it->second->memories.end(); ++it1){
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

void print_processes(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator it, std::ostream &out){
    out << "Processes count: " << it->second->processes.size() << "\n";
    for (auto it1 = it->second->processes.begin(); it1 != it->second->processes.end(); ++it1){
        out << "  name " << it1->second->name.index_ << "\n";
        out << "  root_case\n"; //Does not print (ToDo)
        //for (auto &it3 : it1->second->root_case.actions){
        //    out << "    " << (it3.first.as_wire())->name.index_ << "\n";
        //}
        print_syncs(it1, out);
        out << "\n";
    }
}

void print_ports(Yosys::hashlib::dict<Yosys::RTLIL::IdString, Yosys::RTLIL::Module*>::iterator it, std::ostream &out){
    out << "Ports count: " << it->second->ports.size() << "\n";
    for (auto &it1 : it->second->ports){
        out << "  port " << it1.index_ << "\n";
    }
}

void print_parsed(Yosys::RTLIL::Design &des, std::ostream &out){
    for (auto it=des.modules_.begin(); it != des.modules_.end(); ++it){
        print_modules(it, Yosys::RTLIL::IdString::global_id_index_, out);
        out << "\n";
        print_wires(it, out);
        out << "\n";
        print_cells(it, out);
        out << "\n" ;
        print_connections(it, out);
        out << "\n";
        print_memory(it, out);
        out << "\n";
        print_processes(it, out);
        out << "\n";
        print_ports(it, out);
        out << "\n";

        out << "Refcount wires count: " << it->second->refcount_wires_ << "\n";
        out << "Refcount cells count: " << it->second->refcount_cells_ << "\n";
        out << "Monitors count: " << it->second->monitors.size() << "\n";
        out << "Avail_parameters count: " << it->second->avail_parameters.size() << "\n";
    }
}

int main(int argc, char *argv[]){

    std::ostream& out = std::cout;

    for (size_t a = 1; a<argc; a++){
        std::ifstream f(argv[a]);

        if (f.is_open()){
            out << "\n" << "Opened " << a << " file..." << "\n" << "\n";
        }
        out << argv[a];
        std::vector<std::string> args;
        Yosys::RTLIL::Design design;

        //Yosys::yosys_setup();
        //Yosys::Pass::init_register();
        //out << "Start";
        //Yosys::run_frontend(argv[a], "auto", &design);
        //out << "End";
        parse(f, argv[a], &design, args);
        print_parsed(design, out);
        f.close();
    }
    return 0;
}

