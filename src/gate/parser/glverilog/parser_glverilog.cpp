//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "header_file"
#include "parser.h"
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
  bool correct = 1;

  bool hasSignal(const std::string &value) {
    return std::find(
             variables.begin(), variables.end(), value) != variables.end();
  }

  void hasError () {
    correct = 0;
  }
};

struct SymbolTable {
  struct Symbol {
    FamilyInfo parent;
    FamilyInfo child;
    std::string parentName;
    std::string childName;
    int hasIn = 0;
    int hasOut = 0;
    bool isVareableAlreadyUsed() {
    return hasIn != 0 || hasOut != 0;
  }
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
    void setInUsing(std::string elementName) {
    auto it = table.find(elementName);
    it->second.hasIn++;
  }

  void setOutUsing(std::string elementName) {
    auto it = table.find(elementName);
    it->second.hasOut++;
  }

  bool isOutUsed(std::string elementName) {
    auto it = table.find(elementName);
    return it->second.hasOut != 0;
  }

  bool isInUsed(std::string elementName) {
    auto it = table.find(elementName);
    return it->second.hasIn != 0;
  }

  void findUnused(std::string currentModuleName) {
    
    
    for (auto &element : table) {
      if(element.second.parentName != currentModuleName)
        continue;
      switch (element.second.child) {
    case INPUT_:
      if (element.second.hasIn == 1) {
        std::cerr << "Warning!, this signal is declorated but never used: " 
                  << element.first<< std::endl;
      }
      break;
    case OUTPUT_:
      if (element.second.hasOut == 1) {
        std::cerr << "Warning!, this signal is declorated but never used: " 
                  << element.first<< std::endl;
      }
      break;
    default:
      break;
    }
      
    }
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
       modules[familyNames].hasError();
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
      modules[familyNames].hasError();
    }
    break;
  case FUNC_INI_:
    if (!isVariableAlreadyDeclInChildLevel(table, familyNames, name)) {
      table.changeChildName(yytext, familyNames);

    } else {
      std::cerr << "This variable: " << name 
                << " declarated twice" << std::endl
                << "line: " << yylineno << std::endl;
      modules[familyNames].hasError();
    }
    break;
  case LOGIC_GATE_:
    if (!isVariableAlreadyDeclInParentLevel(table, familyNames, name)) {
      std::cerr << "This variable wasn't declorated in this module: " << name 
                << std::endl << "line: " << yylineno << std::endl;
      modules[familyNames].hasError();
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
    if (symbol.child == INPUT_ && symbol.parent != LOGIC_GATE_) {
      Gate::Id bb = currentNet->net->addIn();
      gates.insert(std::make_pair(entry.first, bb));
      symbolTable.setInUsing(entry.first);
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
    if (symbol.parent == FamilyInfo::LOGIC_GATE_) {
      std::vector<Signal> ids;
      auto arg = gates[modules[it->first].variables.front()];      
      auto type = symbol.child;
      auto dffArg =  gates[modules[it->first].variables[1]];
       std::string outSignal;
      switch (type) {
      case DFF_:
        // ids.push_back(
        //   Signal::always(gates[modules[it->first].variables.front()]));
          symbolTable.setInUsing(modules[it->first].variables.front());
        // ids.push_back(
        //   Signal::always(gates[modules[it->first].variables.back()]));
          symbolTable.setInUsing(modules[it->first].variables.back());
        currentNet->net->setDff(dffArg,gates[modules[it->first].variables.back()],gates[modules[it->first].variables.front()]);
        outSignal = modules[it->first].variables[1];
        break;
      default:
        for (auto idsIt = modules[it->first].variables.begin() + 1;
             idsIt != modules[it->first].variables.end(); ++idsIt) {
        auto fId = gates.find(*idsIt);
        ids.push_back(Signal::always(fId->second));
        symbolTable.setInUsing(*idsIt);
        }
        currentNet->net->setGate(arg, setType(type), ids);
        outSignal = modules[it->first].variables[0];
        break;
      }      
       symbolTable.setOutUsing(outSignal);
    }
  }
   
  for (auto &entry : symbolTable.table) {
    SymbolTable::Symbol &symbol = entry.second;
    if (symbol.parentName != currentModuleName) {
      continue;
    }
    if (symbol.child == FamilyInfo::OUTPUT_) {
      currentNet->net->addOut(gates[entry.first]);   
      symbolTable.setOutUsing(entry.first);
    }   
  }
  
  for (auto &element : symbolTable.table) {  
        if (element.second.parent != WIRE_)
          continue;
        if (symbolTable.isInUsed(element.first) && !symbolTable.isOutUsed(element.first)) {
          std::cerr << "This wire never been used like out: " << element.first
                      << std::endl << "line: " << yylineno <<std::endl;
          modules[currentModuleName].hasError();
        }
        
      }
  symbolTable.findUnused(currentModuleName);
}

bool parseGateLevelVerilog(const std::string &path, 
                           std::vector<std::unique_ptr<GNet>> &nets) {
  yyin = freopen(path.c_str(), "r", stdin);
  if (!yyin) {
      std::cerr << "Error: could not open file " << path << std::endl;
      return false;
  }
  Token_T tok = START;
  KindOfError rc  = SUCCESS;
  SymbolTable table;
  std::unordered_map<std::string, GNetInfo> gnets;
  std::unordered_map<std::string, ModuleInfo> modules;
  while (tok != EOF_TOKEN && rc == SUCCESS) {
    tok = getNextToken();
    if(tok == MODULE) {
      rc = parseModule(tok, table, modules, gnets);
    } else if(tok == EOF_TOKEN) {
        break;
    } else {
        std::cerr << "Error: "<< FAILURE_IN_GATE_LEVEL_VERILOG
                    << std::endl << "line: " << yylineno << std::endl;
        rc = FAILURE_IN_GATE_LEVEL_VERILOG;
    }
   
  if(rc != SUCCESS) {
    return false;
    break;
  }
  }
   if (rc == SUCCESS) {
  for (const auto &gnet_entry : gnets) {
      
      nets.push_back(std::make_unique<GNet>(*gnet_entry.second.net));    
      }
    }
  return rc == SUCCESS;
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
    modules[currentModuleName].hasError();
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
  if (modules[currentModuleName].correct == 0) {
    rc = FAILURE_IN_GATE_LEVEL_VERILOG;
  } 
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
    modules[currentModuleName].hasError();
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

  auto out = currentLogicGate->variables.begin();
  switch (familyType)
  {
  case DFF_:
    out = currentLogicGate->variables.begin() + 1;
    break;
  
  default:
    break;
  }
  for (auto it = currentLogicGate->variables.begin();
           it != currentLogicGate->variables.end(); ++it) { 
    if(it != out && (table.findChild(*it) != INPUT_ 
       && table.findParent(*it) != WIRE_)) {
      std::cerr << "This variable is not input sygnal: " << *it << std::endl
                << "line: " << yylineno << std::endl;
      modules[currentModuleName].hasError();
    } else if (it == out && (table.findChild(*it) != OUTPUT_ 
               && table.findParent(*it) != WIRE_)) {
      std::cerr << "This variable is not output sygnal: " << *it << std::endl
                << "line: " << yylineno << std::endl;
      modules[currentModuleName].hasError();
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
      } 
      break;
    }
  }
  return rc;
}

