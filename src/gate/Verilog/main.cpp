#include "frontends/verilog/verilog_frontend.h"
#include "frontends/verilog/preproc.h"
#include "kernel/yosys.h"
#include "kernel/register.h"
#include "libs/sha1/sha1.h"
#include "frontends/ast/ast.h"
#include <stdarg.h>
#include <iostream>
#include <istream>

YOSYS_NAMESPACE_BEGIN
using namespace VERILOG_FRONTEND;

// use the Verilog bison/flex parser to generate an AST and use AST::process() to convert it to RTLIL

static std::vector<std::string> verilog_defaults;
static std::list<std::vector<std::string>> verilog_defaults_stack;

//static void error_on_dpi_function(Yosys::AST::AstNode *node)
//{
//    if (node->type == Yosys::AST::AST_DPI_FUNCTION)
//        Yosys::log_file_error(node->filename, node->location.first_line, "Found DPI function %s.\n", node->str.c_str());
//	for (auto child : node->children)
//		error_on_dpi_function(child);
//}

static void add_package_types1(dict<std::string, AST::AstNode *> &user_types, std::vector<AST::AstNode *> &package_list)
{
	// prime the parser's user type lookup table with the package qualified names
	// of typedefed names in the packages seen so far.
	for (const auto &pkg : package_list) {
        //log_assert(pkg->type==AST::AST_PACKAGE);
		for (const auto &node: pkg->children) {
            if (node->type == Yosys::AST::AST_TYPEDEF) {
				std::string s = pkg->str + "::" + node->str.substr(1);
				user_types[s] = node;
			}
		}
	}
}

YOSYS_NAMESPACE_END

// the yyerror function used by bison to report parser errors

//void frontend_verilog_yyerror(char const *fmt, ...)
//{
//	va_list ap;
//	char buffer[1024];
//	char *p = buffer;
//	va_start(ap, fmt);
//	p += vsnprintf(p, buffer + sizeof(buffer) - p, fmt, ap);
//	va_end(ap);
//	p += snprintf(p, buffer + sizeof(buffer) - p, "\n");
//	YOSYS_NAMESPACE_PREFIX log_file_error(YOSYS_NAMESPACE_PREFIX AST::current_filename, frontend_verilog_yyget_lineno(),
//					      "%s", buffer);
//	exit(1);
//}

void parse(std::string filename, std::vector<std::string> args){
    std::ifstream f(filename); // open file for reading
    if (f.is_open()){
        std::cout<<"Opened file..."<<std::endl<<std::endl;
    }
    Yosys::RTLIL::Design des;
    Yosys::RTLIL::Design *design=&des;
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

    //Yosys::Pass::extra_args(f, filename, args, argidx);


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

    // make package typedefs available to parser
    Yosys::add_package_types1(Yosys::VERILOG_FRONTEND::pkg_user_types, design->verilog_packages);

    Yosys::VERILOG_FRONTEND::UserTypeMap global_types_map;
    for (auto def : design->verilog_globals) {
        if (def->type == Yosys::AST::AST_TYPEDEF) {
            global_types_map[def->str] = def;
        }
    }

    // use previous global typedefs as bottom level of user type stack
    Yosys::VERILOG_FRONTEND::user_type_stack.push_back(std::move(global_types_map));
    // add a new empty type map to allow overriding existing global definitions
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

    //if (flag_nodpi)
        //error_on_dpi_function(current_ast);

    Yosys::AST::process(design, Yosys::VERILOG_FRONTEND::current_ast, flag_dump_ast1, flag_dump_ast2, flag_no_dump_ptr, flag_dump_vlog1, flag_dump_vlog2, flag_dump_rtlil, flag_nolatches,
            flag_nomeminit, flag_nomem2reg, flag_mem2reg, flag_noblackbox, Yosys::VERILOG_FRONTEND::lib_mode, flag_nowb, flag_noopt, flag_icells, flag_pwires, flag_nooverwrite, flag_overwrite, flag_defer, Yosys::VERILOG_FRONTEND::default_nettype_wire);


    if (!flag_nopp)
        delete Yosys::VERILOG_FRONTEND::lexin;

    // only the previous and new global type maps remain
    Yosys::VERILOG_FRONTEND::user_type_stack.clear();

    delete Yosys::VERILOG_FRONTEND::current_ast;
    Yosys::VERILOG_FRONTEND::current_ast = NULL;


    std::cout<<"hashidx_  "<<des.hashidx_<<std::endl;
    std::cout<<"refcount_modules_  "<<des.refcount_modules_<<std::endl;
    std::cout<<"selected_active_module "<<des.selected_active_module<<std::endl;
    std::cout<<"scratchpad:"<<std::endl;
    for(auto it = des.scratchpad.begin(); it != des.scratchpad.end(); ++it)
    {
        std::cout << it->first << " " << it->second<< std::endl;
    }
    std::cout<<"Monitors:"<<std::endl;

    for (auto i: des.monitors)
        std::cout << i->hashidx_<< ' ';
    std::cout<<"Modules:"<<std::endl;

    for(auto it = des.modules_.begin(); it != des.modules_.end(); ++it)
    {
        std::cout<<"global_id_storage_: "<<std::endl;
        for (char* i: it->first.global_id_storage_)
            std::cout << i << " ";
        std::cout<<"global_id_index_: "<<std::endl;
                for(auto it1 = it->first.global_id_index_.begin(); it1 != it->first.global_id_index_.end(); ++it1)
                {
                    std::cout << it1->first << " " << it1->second<< std::endl;
                }
        std::cout<<"refcount_wires_: "<<it->second->refcount_wires_<<std::endl;
        std::cout<<"refcount_cells_: "<<it->second->refcount_cells_<<std::endl;
        for (auto it2 = it->second->wires_.begin(); it2 != it->second->wires_.end(); ++it2){
            std::cout<<"width: "<<it2->second->width<<std::endl;
            std::cout<<"start_offset: "<<it2->second->start_offset<<std::endl;
            std::cout<<"port_id: "<<it2->second->port_id<<std::endl;
            std::cout<<"port_input: "<<it2->second->port_input<<std::endl;
            std::cout<<"port_output: "<<it2->second->port_output<<std::endl;
            std::cout<<"upto: "<<it2->second->upto<<std::endl;
        }
        std::cout <<"memories:"<<std::endl;
        for (auto it2 = it->second->memories.begin(); it2 != it->second->memories.end(); ++it2){
            std::cout<<"width: "<<it2->second->width<<std::endl;
            std::cout<<"start_offset: "<<it2->second->start_offset<<std::endl;
            std::cout<<"size: "<<it2->second->size<<std::endl;
        }
//        std::cout <<"processes: syncs:"<<std::endl;
//        for (auto it2 = it->second->processes.begin(); it2 != it->second->processes.end(); ++it2){
//            for (char* i: it2->second->syncs)
//                std::cout << i.type << std::endl;
//        }

    }
    std::cout << "selection stack:" << std::endl;
    for (auto i: des.selection_stack){
        std::cout << i.full_selection << '\n';
        std::cout <<"selected_modules & selected_members\n";
//        for (auto j: i.selected_modules){ }
        std::cout << "Look up at the field IdString\n";
    }
}

int main(){

    std::string filename;
    std::cin>>filename;
    std::vector<std::string> args;
    parse(filename, args);

    return 0;
}

