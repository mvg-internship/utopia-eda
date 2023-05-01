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

  auto findInVector(const std::vector<std::string> &vec,
                    const std::string &value) {
    return std::find(vec.begin(), vec.end(), value);
  }
};

struct SymbolTable {
  struct Symbol {
    FamilyInfo parent;
    FamilyInfo child;
    std::string parentName;
    std::string childName;
    int bit;
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
    symbol.bit = 1;
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

  void setBit(const std::string &name, int newbit) {
    table[name].bit = newbit;
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

  int getBit(const std::string &name) {
    return table[name].bit;
  }
};

void assertVariable(const std::string &name, 
                    FamilyInfo familyType,
                    SymbolTable &table,
                    std::string familyNames,
                    std::unordered_map<std::string, ModuleInfo> &modules,
                    int &bitCounter,
                    FamilyInfo assignType) {
  switch (familyType) {
  case FUNCTION_:
    if (modules[familyNames].findInVector(
        modules[familyNames].variables,name) !=
        modules[familyNames].variables.end() &&
        table.findParentName(name) != familyNames) {
      table.changeParentName(name, familyNames);
      table.changeChild(name, VOID_);
    } else {
      std::cerr << "This variable didn't declaration in function: " << name
                << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case WIRE_:
  case MODULE_:
    if (table.findParentName(name) != familyNames) {
      table.addSymbol(name, familyType, FamilyInfo::VOID_);
      table.changeParentName(name, familyNames);
      table.setBit(name, bitCounter);
      std::cout << " parent name: " << table.findParentName(yytext)
                << std::endl;
      modules[familyNames].variables.push_back(name);
    } else {
      std::cerr << "This variable declarated twice:  " << name << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case INPUT_:
  case OUTPUT_:
    if ((table.findParent(name) == MODULE_ ||
         table.findParent(name) == WIRE_) &&
        (table.findChild(name) != INPUT_ && table.findChild(name) != OUTPUT_) 
        && (table.findParentName(name) == familyNames)) {
      table.changeChild(name, familyType);
    } else {
      std::cerr << "Invalid declaration: " << name
                << " parent type: " << table.findParent(name)
                << " parent Name: " << table.findParentName(name)
                << " child type: " << table.findChild(name) << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case ASSIGN_:
    if (table.findParent(name) == MODULE_ &&
        table.findChild(name) == assignType) {
      table.changeChild(name, ASSIGN_);
      bitCounter++;
      std::cout << "counter = " << bitCounter << std::endl;
    } else {
      std::cerr << "Invalid declaration in ASSIGN: " << name
                << " parent type: " << table.findParent(name)
                << " child type: " << table.findChild(name) << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case FUNC_INI_:
    if (table.findChildName(name) != familyNames) {
      table.changeChildName(yytext, familyNames);
      std::cout << "current fu name: " << familyNames << std::endl;
    } else {
      std::cerr << "This variable: " << name 
                << " declarated twice" << std::endl
                << "line: " << yylineno << std::endl;
    }
    break;
  case LOGIC_GATE_:
    if (table.findParentName(name) != familyNames) {
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
KindOfError parseExpr(Token_T &, 
                      SymbolTable &, 
                      std::string,
                      std::unordered_map<std::string, ModuleInfo> &);
KindOfError parseAssignParts(Token_T &, 
                            SymbolTable &, 
                            std::string, 
                            std::unordered_map<std::string, ModuleInfo> &);
KindOfError parseAssign(Token_T &,
                        SymbolTable &,
                        std::string,
                        std::unordered_map<std::string, ModuleInfo> &);
KindOfError parseLogicGate(Token_T &,
                           SymbolTable &,
                           std::string,
                           std::unordered_map<std::string, ModuleInfo> &);
KindOfError parseArg(Token_T &,
                     SymbolTable &,
                     std::string,
                     std::string,
                     std::unordered_map<std::string, ModuleInfo> &);
KindOfError parseNameList(Token_T &,
                          Token_T,
                          FamilyInfo,
                          SymbolTable &,
                          std::string,
                          std::unordered_map<std::string, ModuleInfo> &,
                          int &bit,
                          FamilyInfo);

#define DEBUGTOKEN(tok, msg)                                                  \
  printf("%s:%d: %s: token '%s' (%d)\n", __FILE__, __LINE__, (msg), yytext,   \
         (tok))

#define ASSERT_NEXT_TOKEN(var, tok, err)                                      \
  do {                                                                        \
    var = getNextToken();                                                     \
    if ((var) != (tok)) {                                                     \
      std::cerr << __FILE__ << __LINE__ << yytext << " tok = " << tok         \
                << std::endl;                                                 \
      std::cerr << "Error (in Assert) = " << (err) << std::endl;              \
      return (err);                                                           \
    }                                                                         \
  } while (0)

static std::size_t place = 0;

Token_T getNextToken() {
  Token_T val = static_cast<Token_T>(scan_token());
  std::cout << " type: " << val << " tok: " << yytext << " pl: " << place
            << " ln: " << yylineno << std::endl;
  place += 1;
  return val;
}

void buildGnet(SymbolTable &symbolTable, 
               std::unordered_map<std::string, GNetInfo> &gnets,
               std::string currentModuleName,
               std::unordered_map<std::string, ModuleInfo> &modules) {
  int counter = 0;
  std::unordered_map<std::string, Gate::Id> gates;
  for (auto &entry : symbolTable.table) {
    std::cout << entry.first << std::endl;
    SymbolTable::Symbol &symbol = entry.second;
    if (symbol.parentName != currentModuleName) {
      continue;
    }
    if (symbol.child == FamilyInfo::INPUT_ && symbol.parent != LOGIC_GATE_) {
      Gate::Id bb = gnets[currentModuleName].net->addIn();
      gates.insert(
          std::make_pair(static_cast<std::string>(entry.first), bb));
        for (auto &element : gnets[currentModuleName].elements) {
          if(element.name == entry.first) {
            element.direction = INPUT_;
            element.sourceGate = bb;
          }
        }
    } else if (symbol.parent != LOGIC_GATE_) {
      gates.insert(
          std::make_pair(static_cast<std::string>(entry.first),
              gnets[currentModuleName].net->newGate()));
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
      auto arg =
          gates[static_cast<std::string>(modules[it->first].variables.front())];      
      auto _type = symbol.child;      
      for (auto idsIt = modules[it->first].variables.begin() + 1;
           idsIt != modules[it->first].variables.end(); ++idsIt) {
        auto fId = gates.find(static_cast<std::string>(*idsIt));
        ids.push_back(Signal::always(fId->second));
      }
      for (int i = 0; i < modules[it->first].counter; i++) {
        it++;
      }
    
      switch (_type) {
      case NOT_:
        gnets[currentModuleName].net->setGate(arg, GateSymbol::NOT, ids);
        std::cout << "NOT case symbol: " 
                  << modules[it->first].variables.front()
                  << " gates value: " << arg << std::endl;
        break;
      case AND_:
       std::cout << "AND case: "
                 << modules[it->first].variables.front() 
                 << " gates value: " << arg << std::endl;
        gnets[currentModuleName].net->setGate(arg, GateSymbol::AND, ids);
        break;
      case NAND_:
        gnets[currentModuleName].net->setGate(arg, GateSymbol::NAND, ids);
        break;
      case NOR_:
        gnets[currentModuleName].net->setGate(arg, GateSymbol::NOR, ids);
        break;
      case XOR_:
        gnets[currentModuleName].net->setGate(arg, GateSymbol::XOR, ids);
        break;
      default:
        break;
      }
    }
    }
  
  
  std::cout << "All variables in/out: " << std::endl;
  for (auto &entry : symbolTable.table) {
    SymbolTable::Symbol &symbol = entry.second;
    if (symbol.parentName != currentModuleName) {
      continue;
    }
    if (symbol.child == FamilyInfo::OUTPUT_) {
      Gate::Id bb = gnets[currentModuleName].net->addOut(
          gates[static_cast<std::string>(entry.first)]);;   
      for( auto &element : gnets[currentModuleName].elements) {
          if(element.name == entry.first) {
            element.direction = OUTPUT_;
            element.sourceGate = bb;
           
          }
        }
    }   
  }
  for (auto &element : gnets[currentModuleName].elements) {         
            std::cout << "Element derection: "<<element.direction;
            std::cout << " Element name: "<< element.name<< std::endl;  
        
        }
         std::cout << "Counter 2: " << counter << std::endl;
}

KindOfError parseGateLevelVerilog() {
  Token_T tok = START;
  KindOfError rc  = SUCCESS;
  SymbolTable table;
  std::unordered_map<std::string, GNetInfo> gnets;
  std::unordered_map<std::string, ModuleInfo> modules;
  while (tok != EOF_TOKEN) {
    DEBUGTOKEN(tok, "Verilog loop");
    tok = getNextToken();
    if(tok == MODULE) {
      rc = parseModule(tok, table, modules, gnets);
    }else if(tok == EOF_TOKEN) {
      std::cout << " End of file. Check 'errors.txt' for more info about errors" << std::endl;
      break;
    } else {
      std::cerr << "Error: "<< FAILURE_IN_GATE_LEVEL_VERILOG << std::endl << "line: " << yylineno << std::endl;
    }
  }
  std::cout << "module A: "<< std::endl << *gnets["A"].net<< std::endl;
  std::cout << "module B: " << std::endl << *gnets["B"].net<< std::endl;
  return rc;
}

KindOfError
parseModule(Token_T &tok, SymbolTable &table,
             std::unordered_map<std::string, ModuleInfo> &modules,
             std::unordered_map<std::string, GNetInfo> &gnets) {
  KindOfError rc = SUCCESS;
  int bit = 1;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME); 
  std::string currentModuleName = yytext;
  GNetInfo new_gnet_info;
  new_gnet_info.net = std::make_unique<GNet>();
  gnets[currentModuleName] = std::move(new_gnet_info);
  std::cout << "New GNet instance created for module: " << currentModuleName 
          << ". Address: " << gnets[currentModuleName].net.get() << std::endl;

  if (modules.find(currentModuleName) == modules.end()) { 
    modules[currentModuleName] = {FamilyInfo::MODULE_, {}};
    ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_MODULE_NAME);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
    table.addSymbol(yytext, MODULE_);
    table.changeParentName(yytext, currentModuleName);
    modules[currentModuleName].variables.push_back(yytext);
    std::cout << " parent name: " << table.findParentName(yytext) << std::endl;
    rc = parseNameList(
        tok, RBRACE, MODULE_, table, currentModuleName, modules, bit, VOID_);
    for (auto i = modules[currentModuleName].variables.begin();
        i != modules[currentModuleName].variables.end(); 
        i++) {
      std::string namesOfElementsInThisModule = static_cast<std::string>(*i);
      gnets[currentModuleName].elements.push_back(
          {namesOfElementsInThisModule, VOID_, 0});
    }
    ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_NAME);
    tok = getNextToken();
  } else if (modules[currentModuleName].type == FUNCTION_) {
    std::cout << " Type in modules: " << modules[yytext].type << std::endl;
    ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_MODULE_NAME);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
    std::cout << " The first variable " << yytext << std::endl;
    assertVariable(
        yytext, FUNCTION_, table, currentModuleName, modules, bit, VOID_);
    table.changeParentName(yytext, currentModuleName);
    std::cout << " parent name: " << table.findParentName(yytext) << std::endl;
    rc = parseNameList(
        tok, RBRACE, FUNCTION_, table, currentModuleName, modules, bit, VOID_);
    ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_NAME);
    tok = getNextToken();
  } else {
    std::cerr << "Module name " << yytext << " using twice" << std::endl;
    exit(EXIT_FAILURE);
  }
  while (rc == SUCCESS && tok != ENDMODULE) {
    DEBUGTOKEN(tok, "Module loop");
    switch (tok) {
    case INPUT:
    case OUTPUT:
    case WIRE:
      rc = parseDecl(tok, table, currentModuleName, modules);
      break;
    case ASSIGN:
      rc = parseAssign(tok, table, currentModuleName, modules);
      break;
    case STRING:
      rc = parseExpr(tok, table, currentModuleName, modules);
      break;
    case NOT:
    case NAND:
    case AND:
    case XOR:
    case NOR:
      rc = parseLogicGate(tok, table, currentModuleName, modules);
      break;
    default:
      rc = FAILURE_IN_MODULE_INCAPTULATION;
      break;
    }
  }
  buildGnet(table, gnets, currentModuleName, modules); 
  std::cout << *gnets[currentModuleName].net;
  return rc;
}

KindOfError parseDecl(Token_T &tok, 
                         SymbolTable &table,
                         std::string currentModuleName,
                         std::unordered_map<std::string, ModuleInfo> &modules) {
  FamilyInfo familyType = FamilyInfo::VOID_;
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
  tok = getNextToken();
  KindOfError rc  = SUCCESS;
  int bit = 1;
  switch (tok) {
  case LBRACKET:
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    bit = atoi(yytext);
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_DECL);
    assertVariable(
      yytext, familyType, table, currentModuleName, modules, bit, VOID_);
    rc = parseNameList(
      tok, SEMICOLON, familyType, table, currentModuleName, modules, bit, VOID_);
    tok = getNextToken();
    break;
  case STRING:
    assertVariable(
        yytext, familyType, table, currentModuleName, modules, bit, VOID_);
    rc = parseNameList(
        tok, SEMICOLON, familyType, table, currentModuleName, modules, bit, VOID_);
    tok = getNextToken();
    break;
  default:
    rc = FAILURE_IN_DECL;
    break;
  }
  return rc;
}

KindOfError parseExpr(Token_T &tok, 
                         SymbolTable &table,
                         std::string currentModuleName,
                         std::unordered_map<std::string, ModuleInfo> &modules) {
  KindOfError rc = SUCCESS;
  std::string currentFuncType = yytext;
  std::cout << " Type of func: " << modules[currentFuncType].type << std::endl;
  //table.addSymbol(currentFuncName, FUNC_INI_, FUNC_INI_);
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  std::string currentFuncName = yytext;
  if (modules.find(yytext) != modules.end()) {
    std::cerr << "This name already exist: " << yytext << std::endl 
              << " line: " << yylineno << std::endl;
  }
  table.addSymbol(currentFuncName,FUNC_INI_NAME_, FUNC_INI_NAME_);
  table.changeParentName(currentFuncName, currentModuleName);
  table.changeChildName(currentFuncName, currentFuncType);
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_EXPR);
  DEBUGTOKEN(tok, "Parse expr begin");
  rc = parseArg(tok, table, currentModuleName, currentFuncName, modules);
  DEBUGTOKEN(tok, "Parse expr end");
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
  switch (tok) {
  case NOT:
    familyType = NOT_;
    break;
  case NAND:
    familyType = NAND_;
    break;
  case AND:
    familyType = AND_;
    break;
  case XOR:
    familyType = XOR_;
    break;
  case NOR:
    familyType = NOR_;
    break;
  default:
    break;
  }
  int bit = 0;
  int counterLogicGates = 1;
  //Token_T tmp = tok;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  currentLogicGateName = yytext;
  std::cout << "Logic Gate name is: "<< currentLogicGateName << std::endl;
  if (modules.find(currentLogicGateName) != modules.end() &&
      modules[currentLogicGateName].area == currentModuleName) {
    std::cerr << "This name alredy exsist: " << currentLogicGateName
              << std::endl
              << "line: " << yylineno << std::endl;
  }
  modules[currentLogicGateName] = {
    FamilyInfo::LOGIC_GATE_, {}, currentModuleName};
  table.addSymbol(currentLogicGateName, LOGIC_GATE_, familyType);
  table.changeParentName(yytext,currentModuleName);
  std::cout << "current area: " << modules[currentLogicGateName].area
            << std::endl;
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_EXPR);
  DEBUGTOKEN(tok, "Parse expr begin");
  while (tok != RBRACE && rc == SUCCESS) {
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ARG);
    if (modules[currentLogicGateName].findInVector(
        modules[currentLogicGateName].variables, yytext) !=
        modules[currentLogicGateName].variables.end()) {
      bit = table.getBit(yytext);
      counterLogicGates++;
    }
    assertVariable(
        yytext, LOGIC_GATE_, table, currentModuleName, modules, bit, familyType);
    modules[currentLogicGateName].variables.push_back(yytext);
    tok = getNextToken();
    DEBUGTOKEN(tok, "ARG  loop");
    if (tok == LBRACKET) {
      ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ARG);
      ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ARG);
      tok = getNextToken();
    }
    if (tok != COMMA && tok != RBRACE) {
      rc = FAILURE_IN_ARG;
    }
  }
  for (auto it = modules[currentLogicGateName].variables.begin();
           it != modules[currentLogicGateName].variables.end(); ++it) { 
    if(table.findChild(*it) != INPUT_ && it != modules[currentLogicGateName].variables.begin()) {
      std::cerr << "This variable is not input sygnal: " << *it << std::endl;
    } else if(it == modules[currentLogicGateName].variables.begin() && table.findChild(*it) != OUTPUT_) {
      std::cerr << "This variable is not output sygnal: " << *it << std::endl;
    }
  }
  modules[currentLogicGateName].counter = bit;
  ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_INCAPTULATION);
  tok = getNextToken();
  DEBUGTOKEN(tok, "Parse expr end");
  return rc;
}

KindOfError
parseAssignParts(Token_T &tok, SymbolTable &table,
                   std::string currentModuleName,
                   std::unordered_map<std::string, ModuleInfo> &modules) {
  KindOfError rc = SUCCESS;
  int bitCheck = 0;
  int bit = 0;
  int bitCounter = 0;
  FamilyInfo assignType;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
  if (table.findParent(yytext) == WIRE_ &&
      table.findChild(yytext) == FamilyInfo::VOID_) {
    table.changeChild(yytext, ASSIGN_);
    bitCheck = table.getBit(yytext);
    std::cerr << "Current bit value: " << bitCheck << std::endl
              << " line: " << yylineno << std::endl;
  } else {
    std::cerr << "Invalid declaration in ASSIGN: " << yytext
              << " parent type: " << table.findParent(yytext)
              << " child type: " << table.findChild(yytext) << std::endl
              << "line: " << yylineno << std::endl;   
  }
  tok = getNextToken();
  switch (tok) {
  case LBRACKET:
    std::cout << "aa" << std::endl;
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ASSIGN);
    bit = atoi(yytext);
    if (bit != bitCheck) {
      std::cerr << "Invalid bit value: " << bit << std::endl
                << " line: " << yylineno << std::endl;
    }
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, EQUALS, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, LFIGURNAYA, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
    assignType = table.findChild(yytext);
    assertVariable(yytext, ASSIGN_, table, currentModuleName, modules, bit,
                    assignType);
    rc = parseNameList(tok, RFIGURNAYA, ASSIGN_, table, currentModuleName,
                         modules, bitCounter, assignType);
    if (bitCounter != bitCheck) {
      std::cerr << "Wrong count of arguments in ASSIGN: " << bitCounter + 1
                << " Expected: " << bit << std::endl
                << " line: " << yylineno << std::endl;
    }
    std::cout << __FILE__ << __LINE__ << yytext << " end of lbracket case "
              << "tok =" << tok << std::endl;
    break;
  case EQUALS:
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
    assignType = table.findChild(yytext);
    assertVariable(yytext, ASSIGN_, table, currentModuleName, modules, bit,
                    assignType);
    std::cout << __FILE__ << __LINE__ << yytext << "end of equals case"
              << "tok =" << tok << std::endl;
    break;
  default:
    break;
  }
  return rc;
}

KindOfError
parseAssign(Token_T &tok, SymbolTable &table, 
             std::string currentModuleName,
             std::unordered_map<std::string, ModuleInfo> &modules) {
  KindOfError rc = SUCCESS;
  while (tok != SEMICOLON) {
    rc = parseAssignParts(tok, table, currentModuleName, modules);
    tok = getNextToken();
    if (tok != COMMA && tok != SEMICOLON) {
      rc = FAILURE_IN_ASSIGN;
    }
  }
  tok = getNextToken();
  return rc;
}

KindOfError parseArg(Token_T &tok, SymbolTable &table,
                        std::string currentModuleName,
                        std::string currentFuncName,
                        std::unordered_map<std::string, ModuleInfo> &modules) {
  KindOfError rc = SUCCESS;
  int bit = 1;
  while (tok != RBRACE && rc == SUCCESS) {
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ARG);
    if (modules[currentModuleName].findInVector(
        modules[currentModuleName].variables, yytext) ==
        modules[currentModuleName].variables.end()) {
      std::cerr << "You can't use this variable hear: " << yytext
                << " parent type: " << table.findParent(yytext)
                << " parent name: " << table.findParentName(yytext)
                << " child type: " << table.findChild(yytext) << std::endl
                << "line: " << yylineno << std::endl;
    }
    assertVariable(
        yytext, FUNC_INI_, table, currentFuncName, modules, bit, VOID_);
    modules[currentFuncName].variables.push_back(yytext);
    tok = getNextToken();
    DEBUGTOKEN(tok, "ARG  loop");
    if (tok == LBRACKET) {
      ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ARG);
      ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ARG);
      tok = getNextToken();
    }
    if (tok != COMMA && tok != RBRACE) {
      rc = FAILURE_IN_ARG;
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
                std::unordered_map<std::string, ModuleInfo> &modules,
                int &bit,
                FamilyInfo assignType) {
  KindOfError rc = SUCCESS;
  while (tok != separate_tok && rc == SUCCESS) {
    tok = getNextToken();
    switch (tok) {
    case COMMA:
      ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_PARSE_NAME_LIST);
      assertVariable(
          yytext, familyType, table, familyNames, modules, bit, assignType);
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