#include "headerFile"
#include "token.h"
#include <algorithm>
#include <fstream>
#include <gate/model/gate.h>
#include <gate/model/gnet.h>
#include <gate/model/gsymbol.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

extern "C" YY_DECL;

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;

struct GNetInfo {
  std::unique_ptr<GNet> net;
  struct  IniType
  {
    std::string name;
    FamilyInfo direction; //In or Out
    Gate::Id sourceGate;
  };
  std::vector<IniType> elements;
};

struct ModuleInfo {
  FamilyInfo type;
  std::vector<std::string> variables;
  std::string area;
  int counter = 0;

  bool hasSignal(const std::string &value) {
    return std::find(variables.begin(), variables.end(), value) != variables.end();
  }
};

struct SymbolTable {
  struct Symbol {
    FamilyInfo parent;
    FamilyInfo child;
    std::string parentName;
    std::string childName;
  };
  std::unordered_map<std::string, Symbol> table;

  void addSymbol(const std::string &name, 
                 FamilyInfo parent = VOID_,
                 FamilyInfo child = VOID_) {
    Symbol symbol;
    symbol.parent = parent;
    symbol.child = child;
    symbol.parentName = "";
    symbol.childName = "";
    table[name] = symbol;
  }

  void removeSymbol(const std::string &name) {
    table.erase(name);
  }

  void clearTable() {
    table.erase(table.begin(), table.end());
  }

  void changeParent(const std::string &name, FamilyInfo parent) {
    table[name].parent = parent;
  }

  void changeChild(const std::string &name, FamilyInfo child) {
    table[name].child = child;
  }

  void changeParentName(const std::string &name, std::string newname) {
    table[name].parentName = newname;
  }

  void changeChildName(const std::string &name, std::string newname) {
    table[name].childName = newname;
  }

  FamilyInfo findParent(const std::string &name) {
    return table[name].parent;
  }

  FamilyInfo findChild(const std::string &name) {
    return table[name].child;
  }

  std::string findParentName(const std::string &name) {
    return table[name].parentName;
  }

  std::string findChildName(const std::string &name) {
    return table[name].childName;
  }
};

bool isVariableAlreadyDeclInParentLevel(SymbolTable &table,
                                      std::string familyNames,
                                      std::string name) {
  return table.findParentName(name) == familyNames;
}

bool isVariableAlreadyDeclInParentLevel(SymbolTable &table,
                                      FamilyInfo familyType,
                                      std::string name) {
  return table.findParent(name) == familyType;
}

bool isVariableAlreadyDeclInChildLevel(SymbolTable &table,
                                      std::string familyNames,
                                      std::string name) {
  return table.findChildName(name) == familyNames;
}

bool isVariableAlreadyDeclInChildLevel(SymbolTable &table,
                                      FamilyInfo familyType,
                                      std::string name) {
  return table.findChild(name) == familyType;
}

void assertVariable(const std::string &name, 
                    FamilyInfo familyType,
                    SymbolTable &table,
                    std::string familyNames,
                    std::unordered_map<std::string, ModuleInfo> &modules) {
  switch (familyType) {
  case WIRE_:
  case MODULE_:
    if (!isVariableAlreadyDeclInParentLevel(table, familyNames, name)) {
      table.addSymbol(name, familyType, FamilyInfo::VOID_);
      table.changeParentName(name, familyNames);
      modules[familyNames].variables.push_back(name);
    } else {
      std::cerr << "This variable declarated twice:  " << name << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case INPUT_:
  case OUTPUT_:
    if (isVariableAlreadyDeclInParentLevel(table, familyNames, name) && 
        isVariableAlreadyDeclInChildLevel(table, VOID_, name)) {
      table.changeChild(name, familyType);
    } else {
      std::cerr << "Invalid declaration: " << name
                << " parent type: " << table.findParent(name)
                << " parent Name: " << table.findParentName(name)
                << " child type: " << table.findChild(name) << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case FUNC_INI_:
    if(!isVariableAlreadyDeclInChildLevel(table, familyNames, name)) {
      table.changeChildName(yytext, familyNames);

    } else {
      std::cerr << "This variable: " << name 
                << " declarated twice" << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case LOGIC_GATE_:
    if (!isVariableAlreadyDeclInParentLevel(table, familyNames, name)) {
      std::cerr << "This variable wasn't declorated in this module: " << name 
                << std::endl << "line: " << yylineno << std::endl;
    }
    break;
  default:
    break;
  }
};

Token_T getNextToken();
KindOfError parseGateLevelVerilog();
KindOfError parseModule(Token_T &,
                        SymbolTable &,
                        std::unordered_map<std::string, ModuleInfo> &,
                        std::unordered_map<std::string, GNetInfo> &);
KindOfError parseDecl(Token_T &, 
                      SymbolTable &,
                      std::string,
                      std::unordered_map<std::string, ModuleInfo> &);
KindOfError parseLogicGate(Token_T &,
                           SymbolTable &,
                           std::string,
                           std::unordered_map<std::string, ModuleInfo> &);
KindOfError parseNameList(Token_T &,
                          Token_T,
                          FamilyInfo,
                          SymbolTable &,
                          std::string,
                          std::unordered_map<std::string, ModuleInfo> &);

#define DEBUGTOKEN(tok, msg)                                                  \
  printf("%s:%d: %s: token '%s' (%d)\n", __FILE__, __LINE__, (msg), yytext,   \
         (tok))

#define ASSERT_NEXT_TOKEN(var, tok, err)                                      \
  do {                                                                        \
    var = getNextToken();                                                     \
    if ((var) != (tok)) {                                                     \
      std::cerr << "Error (in Assert) = " << (err) << std::endl               \
                  << "line: "<< yylineno <<  std::endl;                       \
      return (err);                                                           \
    }                                                                         \
  } while (0)

static std::size_t place = 0;

Token_T getNextToken() {
  Token_T val = static_cast<Token_T>(scan_token());
  // std::cout << "type: " << val << " tok: " << yytext <<
  //           " ln: " << yylineno << std::endl;
  place += 1;
  return val;
}

GateSymbol setType(FamilyInfo type) {
  switch (type) {
      case NOT_:
        return GateSymbol::NOT;
        break;
      case AND_:
         return GateSymbol::AND;
        break;
      case NAND_:
        return GateSymbol::NAND;
        break;
      case NOR_:
        return GateSymbol::NOR;
        break;
      case XOR_:
         return GateSymbol::XOR;
        break;
      case OR_:
         return GateSymbol::OR;
        break;
      case XNOR_:
         return GateSymbol::XNOR;
        break;
      case DFF_:
         return GateSymbol::DFF;
        break;
      default:
        break;
      }
    return GateSymbol::ZERO;
}

FamilyInfo setType(Token_T type) {
  switch (type) {
      case NOT:
        return NOT_;
        break;
      case AND:
         return AND_;
        break;
      case NAND:
        return NAND_;
        break;
      case NOR:
        return NOR_;
        break;
      case XOR:
         return XOR_;
        break;
      case OR:
         return OR_;
        break;
      case XNOR:
         return XNOR_;
        break;
      case DFF:
         return DFF_;
        break;
      case INPUT:
         return INPUT_;
        break;
      case OUTPUT:
         return OUTPUT_;
        break;
      case WIRE:
         return WIRE_;
        break;
      default:
        break;
      }
  return VOID_;
}

void buildGnet(SymbolTable &symbolTable, 
               std::unordered_map<std::string, GNetInfo> &gnets,
               std::string currentModuleName,
               std::unordered_map<std::string, ModuleInfo> &modules) {
  std::unordered_map<std::string, Gate::Id> gates;
  GNetInfo* currentNet = &gnets[currentModuleName];
  for (auto &entry : symbolTable.table) {
    SymbolTable::Symbol &symbol = entry.second;
    if (symbol.parentName != currentModuleName) {
      continue;
    }
    if (symbol.child == FamilyInfo::INPUT_ && symbol.parent != LOGIC_GATE_) {
      Gate::Id bb = currentNet->net->addIn();
      gates.insert(std::make_pair(entry.first, bb));
        for (auto &element : currentNet->elements) {
          if(element.name == entry.first) {
            element.direction = INPUT_;
            element.sourceGate = bb;
          }
        }
    } else if (symbol.parent != LOGIC_GATE_) {
      gates.insert(std::make_pair(entry.first, currentNet->net->newGate()));
    } else {
      continue;
    }
  }

  for (auto it = symbolTable.table.begin(); it != symbolTable.table.end();
       it++) {
    SymbolTable::Symbol &symbol = it->second;
    if (symbol.parentName != currentModuleName) {
      continue;
    }
    if (symbol.parent == FamilyInfo::LOGIC_GATE_ 
        || symbol.parent == FamilyInfo::FUNC_INI_) {
      std::vector<Signal> ids;
      auto arg = gates[modules[it->first].variables.front()];      
      auto type = symbol.child;      
      for (auto idsIt = modules[it->first].variables.begin() + 1;
           idsIt != modules[it->first].variables.end(); ++idsIt) {
        auto fId = gates.find(*idsIt);
        ids.push_back(Signal::always(fId->second));
      }
      for (int i = 0; i < modules[it->first].counter; i++) {
        it++;
      }
       currentNet->net->setGate(arg, setType(type), ids);
    }
    }
   
  for (auto &entry : symbolTable.table) {
    SymbolTable::Symbol &symbol = entry.second;
    if (symbol.parentName != currentModuleName) {
      continue;
    }
    if (symbol.child == FamilyInfo::OUTPUT_) {
      Gate::Id bb = currentNet->net->addOut(
          gates[entry.first]);;   
      for( auto &element : currentNet->elements) {
          if(element.name == entry.first) {
            element.direction = OUTPUT_;
            element.sourceGate = bb;
           
          }
        }
    }   
  }
  std::cout << "All variables in/out: " << std::endl;
  for (auto &element : currentNet->elements) { //Shown of list elements with thouse gates numbers
            std::cout << "Gate name: " << gates[element.name];
            std::cout << " Element name: "<< element.name<< std::endl;  
        }
}

KindOfError parseGateLevelVerilog() {
  Token_T tok = START;
  KindOfError rc  = SUCCESS;
  SymbolTable table;
  std::unordered_map<std::string, GNetInfo> gnets;
  std::unordered_map<std::string, ModuleInfo> modules;
  while (tok != EOF_TOKEN) {
    //DEBUGTOKEN(tok, "Verilog loop"); 
    tok = getNextToken();
    if(tok == MODULE) {
      rc = parseModule(tok, table, modules, gnets);
    }else if(tok == EOF_TOKEN) {
      std::cout 
      << " End of file. Check 'errors.txt' for more info about errors" << std::endl;
      break;
    } else {
      std::cerr << "Error: "<< FAILURE_IN_GATE_LEVEL_VERILOG
                 << std::endl << "line: " << yylineno << std::endl;
    }
  }
  for (const auto &gnet_entry : gnets) {
    std::cout << "Module: " << gnet_entry.first << std::endl;
    std::cout << *gnet_entry.second.net << std::endl;
}

  return rc;
}

KindOfError
parseModule(Token_T &tok, 
            SymbolTable &table,
            std::unordered_map<std::string, ModuleInfo> &modules,
            std::unordered_map<std::string, GNetInfo> &gnets) {
  KindOfError rc = SUCCESS;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME); 
  std::string currentModuleName = yytext;
  GNetInfo new_gnet_info;
  new_gnet_info.net = std::make_unique<GNet>();
  gnets[currentModuleName] = std::move(new_gnet_info);

  if (modules.find(currentModuleName) == modules.end()) { 
    modules[currentModuleName] = {FamilyInfo::MODULE_, {}};
    ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_MODULE_NAME);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
    table.addSymbol(yytext, MODULE_);
    table.changeParentName(yytext, currentModuleName);
    modules[currentModuleName].variables.push_back(yytext);
    rc = parseNameList(
        tok, RBRACE, MODULE_, table, currentModuleName, modules);
    for (auto i = modules[currentModuleName].variables.begin();
        i != modules[currentModuleName].variables.end(); 
        i++) {
      std::string namesOfElementsInThisModule = *i;
      gnets[currentModuleName].elements.push_back(
          {namesOfElementsInThisModule, VOID_, 0});
    }
    ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_NAME);
    tok = getNextToken();
  } else {
    std::cerr << "Module name " << yytext << " using twice" << std::endl;
  }
  while (rc == SUCCESS && tok != ENDMODULE) {
    switch (tok) {
    case INPUT:
    case OUTPUT:
    case WIRE:
      rc = parseDecl(tok, table, currentModuleName, modules);
      break;
    case NOT:
    case NAND:
    case AND:
    case XOR:
    case NOR:
    case OR:
    case XNOR:
    case DFF:
      rc = parseLogicGate(tok, table, currentModuleName, modules);
      break;
    default:
      rc = FAILURE_IN_MODULE_INCAPTULATION;
      break;
    }
  }
  buildGnet(table, gnets, currentModuleName, modules); 
  return rc;
}

KindOfError parseDecl(Token_T &tok, 
                      SymbolTable &table,
                      std::string currentModuleName,
                      std::unordered_map<std::string, ModuleInfo> &modules) {
  FamilyInfo familyType = FamilyInfo::VOID_;
  familyType = setType(tok);
  tok = getNextToken();
  KindOfError rc  = SUCCESS;
  switch (tok) {
  case LBRACKET:
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_DECL);
    assertVariable(
      yytext, familyType, table, currentModuleName, modules);
    rc = parseNameList(
      tok, SEMICOLON, familyType, table, currentModuleName, modules);
    tok = getNextToken();
    break;
  case STRING:
    assertVariable(
        yytext, familyType, table, currentModuleName, modules);
    rc = parseNameList(
        tok, SEMICOLON, familyType, table, currentModuleName, modules);
    tok = getNextToken();
    break;
  default:
    rc = FAILURE_IN_DECL;
    break;
  }
  return rc;
}

KindOfError
parseLogicGate(Token_T &tok, 
                 SymbolTable &table,
                 std::string currentModuleName,
                 std::unordered_map<std::string, ModuleInfo> &modules) {
  KindOfError rc = SUCCESS;
  
  std::string currentLogicGateName;
  FamilyInfo familyType = FamilyInfo::VOID_;
  familyType = setType(tok);
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  currentLogicGateName = yytext;
  ModuleInfo* currentLogicGate = &modules[currentLogicGateName];
  if (modules.find(currentLogicGateName) != modules.end() &&
      currentLogicGate->area == currentModuleName) {
    std::cerr << "This name alredy exsist: " << currentLogicGateName
              << std::endl
              << "line: " << yylineno << std::endl;
  }
  *currentLogicGate = {
    FamilyInfo::LOGIC_GATE_, {}, currentModuleName};
  table.addSymbol(currentLogicGateName, LOGIC_GATE_, familyType);
  table.changeParentName(yytext,currentModuleName);
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_EXPR);
  while (tok != RBRACE && rc == SUCCESS) {
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ARG);
    assertVariable(
        yytext, LOGIC_GATE_, table, currentModuleName, modules);
    currentLogicGate->variables.push_back(yytext); 
    tok = getNextToken();
    if (tok == LBRACKET) {
      ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ARG);
      ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ARG);
      tok = getNextToken();
    }
    if (tok != COMMA && tok != RBRACE) {
      rc = FAILURE_IN_ARG;
    }
  }
  for (auto it = currentLogicGate->variables.begin();
           it != currentLogicGate->variables.end(); ++it) { 
    if(it != currentLogicGate->variables.begin() && (table.findChild(*it) != INPUT_ && table.findParent(*it) != WIRE_)) {
      std::cerr << "This variable is not input sygnal: " << *it << std::endl
                << "line: " << yylineno << std::endl;
    } else if (it == currentLogicGate->variables.begin() && (table.findChild(*it) != OUTPUT_ && table.findParent(*it) != WIRE_)) {
      std::cerr << "This variable is not output sygnal: " << *it << std::endl
                << "line: " << yylineno << std::endl;
    }
  }
  ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_INCAPTULATION);
  tok = getNextToken();
  return rc;
}

KindOfError parseNameList(Token_T &tok,
                Token_T separate_tok,
                FamilyInfo familyType,
                SymbolTable &table, std::string familyNames,
                std::unordered_map<std::string, ModuleInfo> &modules) {
  KindOfError rc = SUCCESS;
  while (tok != separate_tok && rc == SUCCESS) {
    tok = getNextToken();
    switch (tok) {
    case COMMA:
      ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_PARSE_NAME_LIST);
      assertVariable(
          yytext, familyType, table, familyNames, modules);
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
  return parseGateLevelVerilog();
}