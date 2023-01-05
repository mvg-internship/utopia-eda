#include "passes/techmap/libparse.h"
#include "kernel/register.h"
#include "kernel/hashlib.h"
#include<iostream>
#include <vector>
struct token_t {
    char type;
    Yosys::RTLIL::SigSpec sig;
    token_t (char t) : type(t) { }
    token_t (char t, Yosys::RTLIL::SigSpec s) : type(t), sig(s) { }
};
static Yosys::RTLIL::SigSpec parse_func_identifier(Yosys::RTLIL::Module *module, const char *&expr)
{
   // log_assert(*expr != 0);

    int id_len = 0;
    while (('a' <= expr[id_len] && expr[id_len] <= 'z') || ('A' <= expr[id_len] && expr[id_len] <= 'Z') ||
            ('0' <= expr[id_len] && expr[id_len] <= '9') || expr[id_len] == '.' ||
            expr[id_len] == '_' || expr[id_len] == '[' || expr[id_len] == ']') id_len++;



    if (id_len == 1 && (*expr == '0' || *expr == '1'))
        return *(expr++) == '0' ? Yosys::RTLIL::State::S0 : Yosys::RTLIL::State::S1;

    std::string id = Yosys::RTLIL::escape_id(std::string(expr, id_len));


    expr += id_len;
    return module->wires_.at(id);
}
static Yosys::RTLIL::SigSpec create_inv_cell(Yosys::RTLIL::Module *module, Yosys::RTLIL::SigSpec A)
{
    Yosys::RTLIL::Cell *cell = module->addCell(NEW_ID, ID($_NOT_));
    cell->setPort(Yosys::ID::A, A);
    cell->setPort(Yosys::ID::Y, module->addWire(NEW_ID));
    return cell->getPort(Yosys::ID::Y);
}

static Yosys::RTLIL::SigSpec create_xor_cell(Yosys::RTLIL::Module *module, Yosys::RTLIL::SigSpec A, Yosys::RTLIL::SigSpec B)
{
    Yosys::RTLIL::Cell *cell = module->addCell(NEW_ID, ID($_XOR_));
    cell->setPort(Yosys::ID::A, A);
    cell->setPort(Yosys::ID::B, B);
    cell->setPort(Yosys::ID::Y, module->addWire(NEW_ID));
    return cell->getPort(Yosys::ID::Y);
}

static Yosys::RTLIL::SigSpec create_and_cell(Yosys::RTLIL::Module *module, Yosys::RTLIL::SigSpec A, Yosys::RTLIL::SigSpec B)
{
    Yosys::RTLIL::Cell *cell = module->addCell(NEW_ID, ID($_AND_));
    cell->setPort(Yosys::ID::A, A);
    cell->setPort(Yosys::ID::B, B);
    cell->setPort(Yosys::ID::Y, module->addWire(NEW_ID));
    return cell->getPort(Yosys::ID::Y);
}

static Yosys::RTLIL::SigSpec create_or_cell(Yosys::RTLIL::Module *module, Yosys::RTLIL::SigSpec A, Yosys::RTLIL::SigSpec B)
{
    Yosys::RTLIL::Cell *cell = module->addCell(NEW_ID, ID($_OR_));
    cell->setPort(Yosys::ID::A, A);
    cell->setPort(Yosys::ID::B, B);
    cell->setPort(Yosys::ID::Y, module->addWire(NEW_ID));
    return cell->getPort(Yosys::ID::Y);
}
static bool parse_func_reduce(Yosys::RTLIL::Module *module, std::vector<token_t> &stack, token_t next_token)
{
    int top = int(stack.size())-1;

    if (0 <= top-1 && stack[top].type == 0 && stack[top-1].type == '!') {
        token_t t = token_t(0, create_inv_cell(module, stack[top].sig));
        stack.pop_back();
        stack.pop_back();
        stack.push_back(t);
        return true;
    }

    if (0 <= top-1 && stack[top].type == '\'' && stack[top-1].type == 0) {
        token_t t = token_t(0, create_inv_cell(module, stack[top-1].sig));
        stack.pop_back();
        stack.pop_back();
        stack.push_back(t);
        return true;
    }

    if (0 <= top && stack[top].type == 0) {
        if (next_token.type == '\'')
            return false;
        stack[top].type = 1;
        return true;
    }

    if (0 <= top-2 && stack[top-2].type == 1 && stack[top-1].type == '^' && stack[top].type == 1) {
        token_t t = token_t(1, create_xor_cell(module, stack[top-2].sig, stack[top].sig));
        stack.pop_back();
        stack.pop_back();
        stack.pop_back();
        stack.push_back(t);
        return true;
    }

    if (0 <= top && stack[top].type == 1) {
        if (next_token.type == '^')
            return false;
        stack[top].type = 2;
        return true;
    }

    if (0 <= top-1 && stack[top-1].type == 2 && stack[top].type == 2) {
        token_t t = token_t(2, create_and_cell(module, stack[top-1].sig, stack[top].sig));
        stack.pop_back();
        stack.pop_back();
        stack.push_back(t);
        return true;
    }

    if (0 <= top-2 && stack[top-2].type == 2 && (stack[top-1].type == '*' || stack[top-1].type == '&') && stack[top].type == 2) {
        token_t t = token_t(2, create_and_cell(module, stack[top-2].sig, stack[top].sig));
        stack.pop_back();
        stack.pop_back();
        stack.pop_back();
        stack.push_back(t);
        return true;
    }

    if (0 <= top && stack[top].type == 2) {
        if (next_token.type == '*' || next_token.type == '&' || next_token.type == 0 || next_token.type == '(' || next_token.type == '!')
            return false;
        stack[top].type = 3;
        return true;
    }

    if (0 <= top-2 && stack[top-2].type == 3 && (stack[top-1].type == '+' || stack[top-1].type == '|') && stack[top].type == 3) {
        token_t t = token_t(3, create_or_cell(module, stack[top-2].sig, stack[top].sig));
        stack.pop_back();
        stack.pop_back();
        stack.pop_back();
        stack.push_back(t);
        return true;
    }

    if (0 <= top-2 && stack[top-2].type == '(' && stack[top-1].type == 3 && stack[top].type == ')') {
        token_t t = token_t(0, stack[top-1].sig);
        stack.pop_back();
        stack.pop_back();
        stack.pop_back();
        stack.push_back(t);
        return true;
    }

    return false;
}

static Yosys::RTLIL::SigSpec parse_func_expr(Yosys::RTLIL::Module *module, const char *expr)
{
    const char *orig_expr = expr;
    std::vector<token_t> stack;

    while (*expr)
    {
        if (*expr == ' ' || *expr == '\t' || *expr == '\r' || *expr == '\n' || *expr == '"') {
            expr++;
            continue;
        }

        token_t next_token(0);
        if (*expr == '(' || *expr == ')' || *expr == '\'' || *expr == '!' || *expr == '^' || *expr == '*' || *expr == '+' || *expr == '|' || *expr == '&')
            next_token = token_t(*(expr++));
        else
            next_token = token_t(0, parse_func_identifier(module, expr));

        while (parse_func_reduce(module, stack, next_token)) {}
        stack.push_back(next_token);
    }

    while (parse_func_reduce(module, stack, token_t('.'))) {}

#if 0

#endif


    return stack.back().sig;
}

static void create_ff(Yosys::RTLIL::Module *module, Yosys::LibertyAst *node)
{
    Yosys::RTLIL::SigSpec iq_sig(module->addWire(Yosys::RTLIL::escape_id(node->args.at(0))));
    Yosys::RTLIL::SigSpec iqn_sig(module->addWire(Yosys::RTLIL::escape_id(node->args.at(1))));

    Yosys::RTLIL::SigSpec clk_sig, data_sig, clear_sig, preset_sig;
    bool clk_polarity = true, clear_polarity = true, preset_polarity = true;

    for (auto child : node->children) {
        if (child->id == "clocked_on")
            clk_sig = parse_func_expr(module, child->value.c_str());
        if (child->id == "next_state")
            data_sig = parse_func_expr(module, child->value.c_str());
        if (child->id == "clear")
            clear_sig = parse_func_expr(module, child->value.c_str());
        if (child->id == "preset")
            preset_sig = parse_func_expr(module, child->value.c_str());
    }


    for (bool rerun_invert_rollback = true; rerun_invert_rollback;)
    {
        rerun_invert_rollback = false;

        for (auto &it : module->cells_) {
            if (it.second->type == ID($_NOT_) && it.second->getPort(Yosys::ID::Y) == clk_sig) {
                clk_sig = it.second->getPort(Yosys::ID::A);
                clk_polarity = !clk_polarity;
                rerun_invert_rollback = true;
            }
            if (it.second->type == ID($_NOT_) && it.second->getPort(Yosys::ID::Y) == clear_sig) {
                clear_sig = it.second->getPort(Yosys::ID::A);
                clear_polarity = !clear_polarity;
                rerun_invert_rollback = true;
            }
            if (it.second->type == ID($_NOT_) && it.second->getPort(Yosys::ID::Y) == preset_sig) {
                preset_sig = it.second->getPort(Yosys::ID::A);
                preset_polarity = !preset_polarity;
                rerun_invert_rollback = true;
            }
        }
    }

    Yosys::RTLIL::Cell *cell = module->addCell(NEW_ID, ID($_NOT_));
    cell->setPort(Yosys::ID::A, iq_sig);
    cell->setPort(Yosys::ID::Y, iqn_sig);

    cell = module->addCell(NEW_ID, "");
    cell->setPort(Yosys::ID::D, data_sig);
    cell->setPort(Yosys::ID::Q, iq_sig);
    cell->setPort(Yosys::ID::C, clk_sig);

    if (clear_sig.size() == 0 && preset_sig.size() == 0) {
        cell->type = Yosys::stringf("$_DFF_%c_", clk_polarity ? 'P' : 'N');
    }

    if (clear_sig.size() == 1 && preset_sig.size() == 0) {
        cell->type = Yosys::stringf("$_DFF_%c%c0_", clk_polarity ? 'P' : 'N', clear_polarity ? 'P' : 'N');
        cell->setPort(Yosys::ID::R, clear_sig);
    }

    if (clear_sig.size() == 0 && preset_sig.size() == 1) {
        cell->type = Yosys::stringf("$_DFF_%c%c1_", clk_polarity ? 'P' : 'N', preset_polarity ? 'P' : 'N');
        cell->setPort(Yosys::ID::R, preset_sig);
    }

    if (clear_sig.size() == 1 && preset_sig.size() == 1) {
        cell->type = Yosys::stringf("$_DFFSR_%c%c%c_", clk_polarity ? 'P' : 'N', preset_polarity ? 'P' : 'N', clear_polarity ? 'P' : 'N');
        cell->setPort(Yosys::ID::S, preset_sig);
        cell->setPort(Yosys::ID::R, clear_sig);
    }

    //log_assert(!cell->type.empty());
}

static bool create_latch(Yosys::RTLIL::Module *module, Yosys::LibertyAst *node, bool flag_ignore_miss_data_latch)
{
    Yosys::RTLIL::SigSpec iq_sig(module->addWire(Yosys::RTLIL::escape_id(node->args.at(0))));
    Yosys::RTLIL::SigSpec iqn_sig(module->addWire(Yosys::RTLIL::escape_id(node->args.at(1))));

    Yosys::RTLIL::SigSpec enable_sig, data_sig, clear_sig, preset_sig;
    bool enable_polarity = true, clear_polarity = true, preset_polarity = true;

    for (auto child : node->children) {
        if (child->id == "enable")
            enable_sig = parse_func_expr(module, child->value.c_str());
        if (child->id == "data_in")
            data_sig = parse_func_expr(module, child->value.c_str());
        if (child->id == "clear")
            clear_sig = parse_func_expr(module, child->value.c_str());
        if (child->id == "preset")
            preset_sig = parse_func_expr(module, child->value.c_str());
    }

    if (enable_sig.size() == 0 || data_sig.size() == 0) {

        return false;
    }

    for (bool rerun_invert_rollback = true; rerun_invert_rollback;)
    {
        rerun_invert_rollback = false;

        for (auto &it : module->cells_) {
            if (it.second->type == ID($_NOT_) && it.second->getPort(Yosys::ID::Y) == enable_sig) {
                enable_sig = it.second->getPort(Yosys::ID::A);
                enable_polarity = !enable_polarity;
                rerun_invert_rollback = true;
            }
            if (it.second->type == ID($_NOT_) && it.second->getPort(Yosys::ID::Y) == clear_sig) {
                clear_sig = it.second->getPort(Yosys::ID::A);
                clear_polarity = !clear_polarity;
                rerun_invert_rollback = true;
            }
            if (it.second->type == ID($_NOT_) && it.second->getPort(Yosys::ID::Y) == preset_sig) {
                preset_sig = it.second->getPort(Yosys::ID::A);
                preset_polarity = !preset_polarity;
                rerun_invert_rollback = true;
            }
        }
    }

    Yosys::RTLIL::Cell *cell = module->addCell(NEW_ID, ID($_NOT_));
    cell->setPort(Yosys::ID::A, iq_sig);
    cell->setPort(Yosys::ID::Y, iqn_sig);

    if (clear_sig.size() == 1)
    {
        Yosys::RTLIL::SigSpec clear_negative = clear_sig;
        Yosys::RTLIL::SigSpec clear_enable = clear_sig;

        if (clear_polarity == true || clear_polarity != enable_polarity)
        {
            Yosys::RTLIL::Cell *inv = module->addCell(NEW_ID, ID($_NOT_));
            inv->setPort(Yosys::ID::A, clear_sig);
            inv->setPort(Yosys::ID::Y, module->addWire(NEW_ID));

            if (clear_polarity == true)
                clear_negative = inv->getPort(Yosys::ID::Y);
            if (clear_polarity != enable_polarity)
                clear_enable = inv->getPort(Yosys::ID::Y);
        }

        Yosys::RTLIL::Cell *data_gate = module->addCell(NEW_ID, ID($_AND_));
        data_gate->setPort(Yosys::ID::A, data_sig);
        data_gate->setPort(Yosys::ID::B, clear_negative);
        data_gate->setPort(Yosys::ID::Y, data_sig = module->addWire(NEW_ID));

        Yosys::RTLIL::Cell *enable_gate = module->addCell(NEW_ID, enable_polarity ? ID($_OR_) : ID($_AND_));
        enable_gate->setPort(Yosys::ID::A, enable_sig);
        enable_gate->setPort(Yosys::ID::B, clear_enable);
        enable_gate->setPort(Yosys::ID::Y, data_sig = module->addWire(NEW_ID));
    }

    if (preset_sig.size() == 1)
    {
        Yosys::RTLIL::SigSpec preset_positive = preset_sig;
        Yosys::RTLIL::SigSpec preset_enable = preset_sig;

        if (preset_polarity == false || preset_polarity != enable_polarity)
        {
            Yosys::RTLIL::Cell *inv = module->addCell(NEW_ID, ID($_NOT_));
            inv->setPort(Yosys::ID::A, preset_sig);
            inv->setPort(Yosys::ID::Y, module->addWire(NEW_ID));

            if (preset_polarity == false)
                preset_positive = inv->getPort(Yosys::ID::Y);
            if (preset_polarity != enable_polarity)
                preset_enable = inv->getPort(Yosys::ID::Y);
        }

        Yosys::RTLIL::Cell *data_gate = module->addCell(NEW_ID, ID($_OR_));
        data_gate->setPort(Yosys::ID::A, data_sig);
        data_gate->setPort(Yosys::ID::B, preset_positive);
        data_gate->setPort(Yosys::ID::Y, data_sig = module->addWire(NEW_ID));

        Yosys::RTLIL::Cell *enable_gate = module->addCell(NEW_ID, enable_polarity ? ID($_OR_) : ID($_AND_));
        enable_gate->setPort(Yosys::ID::A, enable_sig);
        enable_gate->setPort(Yosys::ID::B, preset_enable);
        enable_gate->setPort(Yosys::ID::Y, data_sig = module->addWire(NEW_ID));
    }

    cell = module->addCell(NEW_ID, Yosys::stringf("$_DLATCH_%c_", enable_polarity ? 'P' : 'N'));
    cell->setPort(Yosys::ID::D, data_sig);
    cell->setPort(Yosys::ID::Q, iq_sig);
    cell->setPort(Yosys::ID::E, enable_sig);

    return true;
}

void parse_type_map(std::map<std::string, std::tuple<int, int, bool>> &type_map, Yosys::LibertyAst *ast)
{
    if (ast->children.size()!=0){
    for (auto type_node : ast->children)
    {
        if (type_node->id != "type" || type_node->args.size() != 1)
            continue;

        std::string type_name = type_node->args.at(0);
        int bit_width = -1, bit_from = -1, bit_to = -1;
        bool upto = false;

        for (auto child : type_node->children)
        {
            if (child->id == "base_type" && child->value != "array")
                goto next_type;

            if (child->id == "data_type" && child->value != "bit")
                goto next_type;

            if (child->id == "bit_width")
                bit_width = atoi(child->value.c_str());

            if (child->id == "bit_from")
                bit_from = atoi(child->value.c_str());

            if (child->id == "bit_to")
                bit_to = atoi(child->value.c_str());

            if (child->id == "downto" && (child->value == "0" || child->value == "false" || child->value == "FALSE"))
                upto = true;
        }



        type_map[type_name] = std::tuple<int, int, bool>(bit_width, std::min(bit_from, bit_to), upto);
    next_type:;
    }}
}


void createRTLIL(Yosys::RTLIL::Design *design, std::ifstream &in){
std::vector<std::string> args;
bool flag_lib = false;
bool flag_wb = false;
bool flag_nooverwrite = false;
bool flag_overwrite = false;
bool flag_ignore_miss_func = false;
bool flag_ignore_miss_dir  = false;
bool flag_ignore_miss_data_latch = false;
size_t argidx;
std::vector<std::string> attributes;
Yosys::LibertyParser parser(in);
 int cell_count = 0;
 std::map<std::string, std::tuple<int, int, bool>> global_type_map;
 parse_type_map(global_type_map, parser.ast);
 if (parser.ast->children.size()!=0){
 for (auto cell : parser.ast->children)
 {
     if (cell->id != "cell" || cell->args.size() != 1)
         continue;
     std::string cell_name = Yosys::RTLIL::escape_id(cell->args.at(0));
     if (design->has(cell_name)) {
         Yosys::Module *existing_mod = design->module(cell_name);
         if (!flag_nooverwrite && !flag_overwrite && !existing_mod->get_bool_attribute(Yosys::ID::blackbox)) {
            // log_error("Re-definition of cell/module %s!\n", log_id(cell_name));
         } else if (flag_nooverwrite) {
             //log("Ignoring re-definition of module %s.\n", log_id(cell_name));
             continue;
         } else {
            // log("Replacing existing%s module %s.\n", existing_mod->get_bool_attribute(Yosys::ID::blackbox) ? " blackbox" : "", log_id(cell_name));
             design->remove(existing_mod);
         }
     }
     // log("Processing cell type %s.\n", Yosys::RTLIL::unescape_id(cell_name).c_str());
     std::map<std::string, std::tuple<int, int, bool>> type_map = global_type_map;
     parse_type_map(type_map, cell);

     Yosys::RTLIL::Module *module = new Yosys::RTLIL::Module;
     module->name = cell_name;

     if (flag_lib)
         module->set_bool_attribute(Yosys::ID::blackbox);

     if (flag_wb)
         module->set_bool_attribute(Yosys::ID::whitebox);

     for (auto &attr : attributes)
         module->attributes[attr] = 1;

     for (auto node : cell->children)
     {
         if (node->id == "pin" && node->args.size() == 1) {
             Yosys::LibertyAst *dir = node->find("direction");
             if (!dir || (dir->value != "input" && dir->value != "output" && dir->value != "inout" && dir->value != "internal"))
             {
                 if (!flag_ignore_miss_dir)
                 {
                  //   log_error("Missing or invalid direction for pin %s on cell %s.\n", node->args.at(0).c_str(), log_id(module->name));
                 } else {
                  //   log("Ignoring cell %s with missing or invalid direction for pin %s.\n", log_id(module->name), node->args.at(0).c_str());
                     delete module;
                     goto skip_cell;
                 }
             }
             if (!flag_lib || dir->value != "internal")
                 module->addWire(Yosys::RTLIL::escape_id(node->args.at(0)));
         }

         if (node->id == "bus" && node->args.size() == 1)
         {

              //   log_error("Error in cell %s: bus interfaces are only supported in -lib mode.\n", log_id(cell_name));

             Yosys::LibertyAst *dir = node->find("direction");

             if (dir == nullptr) {
                 Yosys::LibertyAst *pin = node->find("pin");
                 if (pin != nullptr)
                     dir = pin->find("direction");
             }


             if (dir->value == "internal")
                 continue;

             Yosys::LibertyAst *bus_type_node = node->find("bus_type");


             int bus_type_width = std::get<0>(type_map.at(bus_type_node->value));
             int bus_type_offset = std::get<1>(type_map.at(bus_type_node->value));
             bool bus_type_upto = std::get<2>(type_map.at(bus_type_node->value));

             Yosys::Wire *wire = module->addWire(Yosys::RTLIL::escape_id(node->args.at(0)), bus_type_width);
             wire->start_offset = bus_type_offset;
             wire->upto = bus_type_upto;

             if (dir->value == "input" || dir->value == "inout")
                 wire->port_input = true;

             if (dir->value == "output" || dir->value == "inout")
                 wire->port_output = true;
         }
     }

     if (!flag_lib)
     {
         // some liberty files do not put ff/latch at the beginning of a cell
         // try to find "ff" or "latch" and create FF/latch _before_ processing all other nodes
         for (auto node : cell->children)
         {
             if (node->id == "ff" && node->args.size() == 2)
                 create_ff(module, node);
             if (node->id == "latch" && node->args.size() == 2)
                 if (!create_latch(module, node, flag_ignore_miss_data_latch)) {
                     delete module;
                     goto skip_cell;
                 }
         }
     }

     for (auto node : cell->children)
     {
         if (node->id == "pin" && node->args.size() == 1)
         {
             Yosys::LibertyAst *dir = node->find("direction");

             if (flag_lib && dir->value == "internal")
                 continue;

             Yosys::RTLIL::Wire *wire = module->wires_.at(Yosys::RTLIL::escape_id(node->args.at(0)));

             if (dir && dir->value == "inout") {
                 wire->port_input = true;
                 wire->port_output = true;
             }

             if (dir && dir->value == "input") {
                 wire->port_input = true;
                 continue;
             }

             if (dir && dir->value == "output")
                 wire->port_output = true;

             if (flag_lib)
                 continue;

             Yosys::LibertyAst *func = node->find("function");
             if (func == NULL)
             {
                 if (!flag_ignore_miss_func)
                 {
                     //log_error("Missing function on output %s of cell %s.\n", log_id(wire->name), log_id(module->name));
                 } else {
                     //og("Ignoring cell %s with missing function on output %s.\n", log_id(module->name), log_id(wire->name));
                     delete module;
                     goto skip_cell;
                 }
             }

             Yosys::RTLIL::SigSpec out_sig = parse_func_expr(module, func->value.c_str());
             module->connect(Yosys::RTLIL::SigSig(wire, out_sig));
         }
     }

     module->fixup_ports();
     design->add(module);
     cell_count++;
     skip_cell:;
 }

}
}
int main(int argc, char* argv[]){
    for (size_t o=1;o<argc;++o){
    std::ifstream in(argv[o]);
    Yosys::RTLIL::Design des;
    Yosys::RTLIL::Design *design=&des;
    createRTLIL(design, in);
    std::cout<<"hashidx_  "<<des.hashidx_<<"\n";
    std::cout<<"refcount_modules_  "<<des.refcount_modules_<<"\n";
    std::cout<<"selected_active_module "<<des.selected_active_module<<"\n";
    std::cout<<"scratchpad:\n";
    for(auto it = des.scratchpad.begin(); it != des.scratchpad.end(); ++it)
    {
        std::cout << it->first << " " << it->second<< "\n";
    }
    std::cout<<"Monitors:\n";

    for (auto i: des.monitors)
        std::cout << i->hashidx_<< ' ';
    std::cout<<"Modules:\n";

    for(auto it = des.modules_.begin(); it != des.modules_.end(); ++it)
    {
        std::cout<<"global_id_storage_: "<<std::endl;
        for (char* i: it->first.global_id_storage_)
            std::cout << i << ' ';
        std::cout<<"global_id_index_: "<<std::endl;
                for(auto it1 = it->first.global_id_index_.begin(); it1 != it->first.global_id_index_.end(); ++it1)
                {
                    std::cout << it1->first << " " << it1->second<< "\n";
                }
        std::cout<<"refcount_wires_: "<<it->second->refcount_wires_<<"\n";
        std::cout<<"refcount_cells_: "<<it->second->refcount_cells_<<"\n";
        for (auto it2 = it->second->wires_.begin(); it2 != it->second->wires_.end(); ++it2){
            std::cout<<"width: "<<it2->second->width<<"\n";
            std::cout<<"start_offset: "<<it2->second->start_offset<<"\n";
            std::cout<<"port_id: "<<it2->second->port_id<<"\n";
            std::cout<<"port_input: "<<it2->second->port_input<<"\n";
            std::cout<<"port_output: "<<it2->second->port_output<<"\n";
            std::cout<<"upto: "<<it2->second->upto<<"\n";
        }
        std::cout <<"memories: \n";
        for (auto it2 = it->second->memories.begin(); it2 != it->second->memories.end(); ++it2){
            std::cout<<"width: "<<it2->second->width<<"\n";
            std::cout<<"start_offset: "<<it2->second->start_offset<<"\n";
            std::cout<<"size: "<<it2->second->size<<"\n";
        }
//        std::cout <<"processes: syncs: \n";
//        for (auto it2 = it->second->processes.begin(); it2 != it->second->processes.end(); ++it2){
//            for (char* i: it2->second->syncs)
//                std::cout << i.type << '\n ';
//        }

    }
    std::cout << "selection stack: \n";
    for (auto i: des.selection_stack){
        std::cout << i.full_selection << '\n';
        std::cout <<"selected_modules & selected_members\n";
//        for (auto j: i.selected_modules){

//        } уже выведены
        std::cout << "Look up at the field IdString\n";
    }
    }
    return 0;
}


