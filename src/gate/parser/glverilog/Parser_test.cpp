#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
//#include "lex.yy.c"
#include <algorithm>
#include "token.h"
#include "headerFile"
#include <gate/model/gnet.h>
#include <gate/model/gate.h>
#include <gate/model/gsymbol.h>

/*
some defenicion in Verilog

T = {module, endmodule, input, output, wire, not,and, nand, end, or, xor, /n}
N = { =, (, )}

stmt ->  module* EOF
module -> MODULE name "(" name(,name)*")"; decl* expr* | ENDMODULE
decl -> (input|output|wire)?[NUM:NUM]? name(,name)* ;// тут мы будем проверять,
есть ли далее скобка, так как она опциональна expr -> name name (arg*,); arg->
name ?[NUM]?; // тут мы будем проверять, есть ли далее скобка, так как она
опциональна
*/

extern "C" YY_DECL;

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;


struct ModuleInfo {
  familyInfo type;
  std::vector<std::string> variables;
  std::string area;

  auto find_in_vector(const std::vector<std::string>& vec, const std::string& value) {
    return std::find(vec.begin(), vec.end(), value);
  }
};

struct SymbolTable {

  struct Symbol {
    familyInfo  parent;
    familyInfo  child;
    std::string parentName;
    std::string childName;
    int bit;
    Gate::Id gateId;
  };
  std::unordered_map<std::string, Symbol> table;


  void addSymbol(const std::string &name, 
                 familyInfo parent = VOID_,
                 familyInfo child = VOID_) {
    Symbol symbol;
    symbol.parent     = parent;
    symbol.child      = child;
    symbol.parentName;
    symbol.childName;
    symbol.bit = 1;
    table[name]       = symbol;
  }

  void removeSymbol(const std::string &name) {
    table.erase(name);
  }

  void clearTable() {
    table.erase(table.begin(), table.end());
  }

  void changeParent(const std::string &name, familyInfo parent) {
    table[name].parent = parent;
  }

  void changeChild(const std::string &name, familyInfo child) {
    table[name].child = child;
  }

  void changeParentName(const std::string &name, std::string newname) {
    table[name].parentName = newname;
  }

  void changeChildName(const std::string &name, std::string newname) {
    table[name].childName = newname;
  }

  void setBit(const std::string &name, int newbit){
    table[name].bit = newbit;
  }

  familyInfo findParent(const std::string &name) {
    return table[name].parent;
  }

  familyInfo findChild(const std::string &name) {
    return table[name].child;
  }

  std::string findParentName(const std::string &name) {
    return table[name].parentName;
  }

  std::string findChildName(const std::string &name) {
    return table[name].childName;
  }

  int getBit(const std::string &name){
    return table[name].bit;
  }
};

void AsserrtVariable(const std::string &name,
                     familyInfo familyType,
                     SymbolTable &table,
                     std::string familyNames,
                     std::unordered_map<std::string, ModuleInfo> &modules,
                     int& bitCounter,
                     familyInfo assignType) {
  switch (familyType) {
  case FUNCTION_:
  if(modules[familyNames].find_in_vector(modules[familyNames].variables, name)!= modules[familyNames].variables.end() &&
     table.findParentName(name) != familyNames){
    table.changeParentName(name, familyNames);
    table.changeChild(name,VOID_);
  } else {
    std::cerr << "This variable didn't declaration in function: " << name << std::endl
                << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
  }

  break;
  case WIRE_:
  case MODULE_:
    if (table.findParentName(name) != familyNames) {
      table.addSymbol(name, familyType, familyInfo::VOID_);
      table.changeParentName(name, familyNames);
      table.setBit(name, bitCounter);
      std::cout << " parent name: " << table.findParentName(yytext) << std::endl;
      modules[familyNames].variables.push_back(name);
    } else {
      std::cerr << "This variable declarated twice:  " << name << std::endl
                << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
    }

    break;
  case INPUT_:
  case OUTPUT_:
    if ((table.findParent(name) == MODULE_ || table.findParent(name) == WIRE_) && 
        (table.findChild(name) != INPUT_ && table.findChild(name) != OUTPUT_) &&
        (table.findParentName(name) == familyNames)) {
      table.changeChild(name, familyType);
    } else {
      std::cerr << "Invalid declaration: " << name
                << " parent type: " << table.findParent(name)
                << " parent Name: " << table.findParentName(name)
                << " child type: " << table.findChild(name) << std::endl
                << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
    }

    break;
  case ASSIGN_:
    if (table.findParent(name) == MODULE_ && table.findChild(name) == assignType) {
      table.changeChild(name, ASSIGN_);
      bitCounter++;
      std::cout << "counter = "<<bitCounter<<std::endl;
    } else {
      std::cerr << "Invalid declaration in ASSIGN: " << name
                << " parent type: " << table.findParent(name)
                << " child type: " << table.findChild(name) << std::endl
                << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
    }

    break;
  case FUNC_INI_:
    if(table.findChildName(name) != familyNames ) {
      table.changeChildName(yytext, familyNames);
      std::cout <<"current fu name: "<< familyNames << std::endl;
     //std::cout <<"First element of module for this fu: " <<modules[familyNames].variables[0] << std::endl;
    } else {
      std::cerr << "This variable: " << name << " declarated twice" << std::endl << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
      
    }
    break;  
  case LOGIC_GATE_:

    break;
  default:

    break;
  }
};

token_t get_next_token();
kind_of_error parse_gatelevel_verilog();
kind_of_error parse_module(token_t &, SymbolTable &, std::unordered_map<std::string, ModuleInfo> &);
kind_of_error parse_decl(token_t &, SymbolTable &, std::string, std::unordered_map<std::string, ModuleInfo> &);
kind_of_error parse_expr(token_t &, SymbolTable &, std::string, std::unordered_map<std::string, ModuleInfo> &);
kind_of_error parse_assign_parts(token_t &, SymbolTable &, std::string, std::unordered_map<std::string, ModuleInfo> &);
kind_of_error parse_assign(token_t &, SymbolTable &, std::string, std::unordered_map<std::string, ModuleInfo> &);
kind_of_error parse_logic_gate(token_t &, SymbolTable &, std::string, std::unordered_map<std::string, ModuleInfo> &);
kind_of_error parse_arg(token_t &, SymbolTable &, std::string, std::string, std::unordered_map<std::string, ModuleInfo> &);
kind_of_error parse_name_list(token_t &, token_t, familyInfo, SymbolTable &, std::string, std::unordered_map<std::string, ModuleInfo> &, int& bit, familyInfo);

#define DEBUGTOKEN(tok, msg)                                                   \
  printf("%s:%d: %s: token '%s' (%d)\n", __FILE__, __LINE__, (msg), yytext,    \
         (tok))

#define ASSERT_NEXT_TOKEN(var, tok, err)                                       \
  do {                                                                         \
    var = get_next_token();                                                    \
    if ((var) != (tok)) {                                                      \
      std::cout << __FILE__ << __LINE__ << yytext << " tok = " << tok          \
                << std::endl;                                                  \
      std::cout << "Error (in Assert) = " << (err) << std::endl;               \
      return (err);                                                            \
    }                                                                          \
  } while (0)

static std::size_t place = 0;

token_t get_next_token() {
  token_t val = static_cast<token_t>(scan_token());
  std::cout << " type: " << val << " tok: " << yytext << " pl: " << place
            << " ln: " << yylineno << std::endl;
  place += 1;
  return val;
}




//GNet net;

GNet buildGnet(SymbolTable& symbolTable, GNet& net, std::string currentModuleName, std::unordered_map<std::string, ModuleInfo> &modules) {
  for ( auto& entry : symbolTable.table) {
     SymbolTable::Symbol& symbol = entry.second;
    if(symbol.parentName != currentModuleName) {
      break;
    }
    if (symbol.child == familyInfo::INPUT_) {
     symbol.gateId = net.addIn();
    }
  }

  for (auto it = symbolTable.table.begin(); it != symbolTable.table.end(); ++it) {
    const SymbolTable::Symbol& symbol = it->second;
    if(symbol.parentName != currentModuleName) {
      break;
    }

    if(symbol.child == familyInfo::LOGIC_GATE_) {
      std::vector<Signal> ids;
      auto arg = symbol.gateId;
      auto _type = symbol.child;
    
      for(auto idsIt = modules[it->first].variables.begin(); idsIt != modules[it->first].variables.end();++idsIt ) {
        // auto tmp = idsIt->
        symbolTable.table[it->first].gateId ;
        ids.push_back(Signal::always(symbolTable.table[it->first].gateId));
      }
      it++;
      switch (_type) {
      case NOT_:
        net.setGate(arg, GateSymbol::NOT, ids);
        break;
      case AND_:
        net.setGate(arg, GateSymbol::AND, ids);
        break;
      case NAND_:
        net.setGate(arg, GateSymbol::NAND, ids);
        break;
      case NOR_:
        net.setGate(arg, GateSymbol::NOR, ids);
        break;
      case XOR_:
        net.setGate(arg, GateSymbol::XOR, ids);
        break;
      //case
      default:
        break;
      }
    }
     
    // if (symbol.child != familyInfo::INPUT_ && symbol.child != familyInfo::OUTPUT_ && symbol.child == familyInfo::DEFINED) {
    //   std::vector<Signal> ids;
    //   auto arg = symbol.gateId;
    //   auto _type = symbol.child;
    //   //for(auto idsIt = modules[])
    //   for (auto newIt = std::next(it); newIt != symbolTable.table.end() && newIt->second.childName == currentFuncName; ++newIt) {
    //     ids.push_back(Signal::always(newIt->second.gateId));
    //     it = newIt;
    //   }
      
     
    }

  for (const auto& entry : symbolTable.table) {
    const SymbolTable::Symbol& symbol = entry.second;
    if(symbol.parentName != currentModuleName) {
      break;
    }
    if (symbol.child == familyInfo::OUTPUT_) {
      net.addOut(symbol.gateId);
    }
  }
  return net;
}

kind_of_error parse_gatelevel_verilog() {

  token_t tok = START;
  kind_of_error rc = SUCCESS;
  SymbolTable table;
  std::unordered_map<std::string, ModuleInfo> modules;
  while (tok != EOF_TOKEN) {
    DEBUGTOKEN(tok, "Verilog loop");
    ASSERT_NEXT_TOKEN(tok, MODULE, FAILURE_IN_GATE_LEVEL_VERILOG);
    rc = parse_module(tok, table, modules);
  }
  std::cout << "Error! "
            << "type = " << rc << std::endl;
  return rc;
}

kind_of_error parse_module(token_t &tok, SymbolTable &table, std::unordered_map<std::string, ModuleInfo> &modules) {
  kind_of_error rc = SUCCESS;
  int bit = 1;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME); // Имя модуля
  std::string currentModuleName = yytext;
  
  if(modules.find(currentModuleName) == modules.end()) {
    //Старый случай. Ничего не меняется. Единственное что - теперь мы сохраняем этот modulename в нашу новую структуру
    modules[currentModuleName] = {familyInfo::MODULE_, {}};
    ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_MODULE_NAME);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
    table.addSymbol(yytext, MODULE_);
    table.changeParentName(yytext, currentModuleName);
    modules[currentModuleName].variables.push_back(yytext);
    std::cout << " parent name: " << table.findParentName(yytext) << std::endl;
    rc = parse_name_list(tok, RBRACE, MODULE_, table, currentModuleName, modules, bit, VOID_);
    ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_NAME);
    tok = get_next_token();
  } else if(modules[currentModuleName].type == FUNCTION_) {
    //Новый случай
    std::cout << " Type in modules: " << modules[yytext].type << std::endl;
    ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_MODULE_NAME);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
  std::cout << " The first variable " << yytext << std::endl;
    AsserrtVariable(yytext, FUNCTION_, table, currentModuleName, modules, bit, VOID_);
    table.changeParentName(yytext, currentModuleName);
    std::cout << " parent name: " << table.findParentName(yytext) << std::endl;
    rc = parse_name_list(tok, RBRACE, FUNCTION_, table, currentModuleName, modules, bit, VOID_);
    ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_NAME);
    tok = get_next_token();
  } else {
    std::cerr << "Module name "<< yytext <<" using twice"<<std::endl;
    exit(EXIT_FAILURE);
  }
  //table.findChildName(yytext);

  while (rc == SUCCESS && tok != ENDMODULE) {
    DEBUGTOKEN(tok, "Module loop");

    switch (tok) {
    case INPUT:
    case OUTPUT:
    case WIRE:
      rc = parse_decl(tok, table, currentModuleName, modules);
      break;
    case ASSIGN:
      rc = parse_assign(tok, table, currentModuleName, modules);
      break;
    case STRING:
      rc = parse_expr(tok, table, currentModuleName, modules);
      break;
    case NOT:
    case NAND:
    case AND:
    case XOR:
    case NOR:
      rc = parse_logic_gate(tok, table, currentModuleName, modules);
      break;
    default:
      rc = FAILURE_IN_MODULE_INCAPTULATION;
      break;
    }
  }

  //table.clearTable();
  return rc;
}

kind_of_error parse_decl(token_t &tok, SymbolTable &table, std::string currentModuleName, std::unordered_map<std::string, ModuleInfo> &modules) {
  //Сохраняем тип деклорации. Ничего умнее я не придумал
  familyInfo familyType = familyInfo::VOID_;
  switch (tok) {
  case INPUT:
    familyType = INPUT_;
    break;
  case OUTPUT:
    familyType = OUTPUT_;
    break;
  case WIRE:
    familyType = WIRE_;
    break;
  default:
    break;
  }
  tok = get_next_token();
  kind_of_error rc = SUCCESS;
  int bit = 1;
  switch (tok) {
  case LBRACKET:
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    bit = atoi(yytext);
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_DECL);
    AsserrtVariable(yytext, familyType, table, currentModuleName, modules, bit, VOID_);
    rc  = parse_name_list(tok, SEMICOLON, familyType, table, currentModuleName, modules, bit, VOID_);
    tok = get_next_token();

    break;
  case STRING:
    AsserrtVariable(yytext, familyType, table, currentModuleName, modules, bit, VOID_);
    rc = parse_name_list(tok, SEMICOLON, familyType, table, currentModuleName, modules, bit, VOID_);
    tok = get_next_token();

    break;
  default:
    rc = FAILURE_IN_DECL;
    break;
  }

  return rc;
}

kind_of_error parse_expr(token_t &tok, SymbolTable &table, std::string currentModuleName, std::unordered_map<std::string, ModuleInfo> &modules) {
  kind_of_error rc = SUCCESS;
  std::string currentFuncName = yytext;

  //modules[currentFuncName] = {familyInfo::FUNCTION_, {}};
   std::cout << " Type of func: " << modules[currentFuncName].type << std::endl;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_EXPR);
  DEBUGTOKEN(tok, "Parse expr begin");
  rc = parse_arg(tok, table, currentModuleName, currentFuncName, modules);
  DEBUGTOKEN(tok, "Parse expr end");
  return rc;
}

 kind_of_error parse_logic_gate(token_t &tok, SymbolTable &table, std::string currentModuleName, std::unordered_map<std::string, ModuleInfo> &modules) {
  kind_of_error rc = SUCCESS;
  std::string currentLogicGateName;
  int bit = 1;
  int counterLogicGates = 1;
  token_t tmp = tok;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  currentLogicGateName = yytext;
  if(modules.find(currentLogicGateName) != modules.end() && modules[currentLogicGateName].area == currentModuleName) {
    std::cerr << "This name alredy exsist: "<< currentLogicGateName << std::endl << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
  }
  modules[currentLogicGateName] = {familyInfo::LOGIC_GATE_, {}, currentModuleName};
  std::cout << "current area: " << modules[currentLogicGateName].area << std::endl; 
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_EXPR);
  DEBUGTOKEN(tok, "Parse expr begin");
  while (tok != RBRACE && rc == SUCCESS) {
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ARG); 
    if(modules[currentLogicGateName].find_in_vector(modules[currentLogicGateName].variables, yytext) != modules[currentLogicGateName].variables.end()){
      bit = table.getBit(yytext);
      counterLogicGates++;
    }
    
    AsserrtVariable(yytext,LOGIC_GATE_, table, currentModuleName, modules, bit, VOID_);
    modules[currentLogicGateName].variables.push_back(yytext);
    tok = get_next_token();
    DEBUGTOKEN(tok, "ARG  loop");

    if (tok == LBRACKET) {
      ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ARG);
      ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ARG);
      tok = get_next_token();
    }
    if (tok != COMMA && tok != RBRACE) {
      rc = FAILURE_IN_ARG;
    }
  }
  if(bit != counterLogicGates){
      std::cerr << "Count of arguments of this logic gate is wrong: "<< currentLogicGateName << std::endl << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
    }
  ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_INCAPTULATION);
  tok = get_next_token();
  DEBUGTOKEN(tok, "Parse expr end");
  return rc;
}


kind_of_error parse_assign_parts(token_t &tok, SymbolTable &table, std::string currentModuleName, std::unordered_map<std::string, ModuleInfo> &modules) {
  kind_of_error rc = SUCCESS;
  int bitCheck = 0;
  int bit = 0;
  int bitCounter = 0;
  familyInfo assignType;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
  if (table.findParent(yytext) == WIRE_ &&
      table.findChild(yytext) == familyInfo::VOID_) {
    table.changeChild(yytext, ASSIGN_);
    bitCheck = table.getBit(yytext);
    std::cerr << "Current bit value: " << bitCheck << std::endl << " line: " << yylineno << std::endl;
  } else {
    std::cerr << "Invalid declaration in ASSIGN: " << yytext
              << " parent type: " << table.findParent(yytext)
              << " child type: " << table.findChild(yytext) << std::endl
              << "line: " << yylineno << std::endl;
    exit(EXIT_FAILURE);
  }
  tok = get_next_token();
  switch (tok) {
  case LBRACKET:
    std::cout << "aa" << std::endl;
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ASSIGN);
     bit = atoi(yytext);
    if(bit != bitCheck){
      std::cerr << "Invalid bit value: " << bit << std::endl << " line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
    }
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, EQUALS, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, LFIGURNAYA, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
    assignType = table.findChild(yytext);
    AsserrtVariable(yytext, ASSIGN_, table, currentModuleName, modules, bit, assignType);
    rc = parse_name_list(tok, RFIGURNAYA, ASSIGN_, table, currentModuleName, modules, bitCounter, assignType);
    if(bitCounter != bitCheck){
      std::cerr << "Wrong count of arguments in ASSIGN: " << bitCounter + 1 << " Expected: " << bit  << std::endl << " line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
    }
    std::cout << __FILE__ << __LINE__ << yytext << " end of lbracket case "
              << "tok =" << tok << std::endl;
    break;
  case EQUALS:
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
    assignType = table.findChild(yytext);
    AsserrtVariable(yytext, ASSIGN_, table, currentModuleName, modules, bit, assignType);
    std::cout << __FILE__ << __LINE__ << yytext << "end of equals case"
              << "tok =" << tok << std::endl;
    break;
  default:
    break;
  }

  return rc;
}

kind_of_error parse_assign(token_t &tok, SymbolTable &table, std::string currentModuleName, std::unordered_map<std::string, ModuleInfo> &modules) {
  kind_of_error rc = SUCCESS;
  while (tok != SEMICOLON) {
    rc  = parse_assign_parts(tok, table, currentModuleName, modules);
    tok = get_next_token();
    if (tok != COMMA && tok != SEMICOLON) {
      rc = FAILURE_IN_ASSIGN;
    }
  }
  tok = get_next_token();
  return rc;
}

kind_of_error parse_arg(token_t &tok, SymbolTable &table, std::string currentModuleName, std::string currentFuncName, std::unordered_map<std::string, ModuleInfo> &modules) {
  kind_of_error rc = SUCCESS;
  int bit = 1;
  while (tok != RBRACE && rc == SUCCESS) {
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ARG); 
      std::cout <<"First element of module for this fu: " <<modules[currentFuncName].variables[0] << std::endl;
    if(modules[currentModuleName].find_in_vector(modules[currentModuleName].variables, yytext) == modules[currentModuleName].variables.end()){
      std::cerr << "You can't use this variable hear: " << yytext
                << " parent type: " << table.findParent(yytext)
                << " parent name: " << table.findParentName(yytext)
                << " child type: " << table.findChild(yytext) << std::endl
                << "line: " << yylineno << std::endl;
      exit(EXIT_FAILURE);
    }
    
    AsserrtVariable(yytext,FUNC_INI_, table, currentFuncName, modules, bit, VOID_);
    
    tok = get_next_token();
    DEBUGTOKEN(tok, "ARG  loop");

    if (tok == LBRACKET) {
      ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ARG);
      ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ARG);
      tok = get_next_token();
    }
    if (tok != COMMA && tok != RBRACE) {
      rc = FAILURE_IN_ARG;
    }
  }
  ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_INCAPTULATION);
  tok = get_next_token();
  return rc;
}

kind_of_error parse_name_list(token_t &tok,
                              token_t separate_tok,
                              familyInfo familyType, 
                              SymbolTable &table,
                              std::string familyNames,
                              std::unordered_map<std::string, ModuleInfo> &modules,
                              int& bit, 
                              familyInfo assignType) {
  kind_of_error rc = SUCCESS;
  while (tok != separate_tok && rc == SUCCESS) {
    tok = get_next_token();
    switch (tok) {
    case COMMA:
      ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_PARSE_NAME_LIST);
      AsserrtVariable(yytext, familyType, table, familyNames, modules, bit, assignType);
      break;
    default:
      if (tok != separate_tok) {
        rc = FAILURE_IN_PARSE_NAME_LIST;
      } else {
        break;
      }
      break;
    }
  }
  return rc;
}



int main(int argc, char *argv[]) {
  yyin = fopen(argv[1], "r");
  return parse_gatelevel_verilog();
}