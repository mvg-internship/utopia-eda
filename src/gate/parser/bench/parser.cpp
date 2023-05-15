//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/model/gsymbol.h"
extern "C" {
#include "header_file"
}
#include "tokens.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

///define useful cerr-throw message.
#define CERR(message) do {\
  std::cerr << std::endl << "Caught "\
    << benchtext << " line "<< benchlineno << std::endl;\
  throw std::logic_error(message);\
} while (false)

///define useful cerr-throw message.
#define VIEW() do {\
    std::cerr << "exeption token: " << it->first << " ";\
    for (const auto &i : it->second.uses) {\
      std::cerr << "line " << i.line << "\n";\
    }\
} while (false)

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;

struct VarInfo {
  const bool def;
  Tokens typeInit;  
  int line;
};

struct SymbolInfo {
  std::vector<VarInfo> uses;
  Gate::Id gateId;
  std::vector<std::string> args;
};

///Defines the adding in a infos map.
SymbolInfo& addVarInfo(bool def,
                       Tokens typeInit,
                       std::map<std::string, SymbolInfo> &infos) {
  const std::string name {benchtext};
  auto &inf = infos[name];
  const VarInfo map {def, typeInit, benchlineno};
  inf.uses.push_back(map);
  return inf;
}

SymbolInfo& addVarDef(Tokens typeInit,
                      std::map<std::string, SymbolInfo> &infos) {
  return addVarInfo(true, typeInit, infos);
}

SymbolInfo& addVarUse(Tokens typeInit,
                      std::map<std::string, SymbolInfo> &infos) {
  return addVarInfo(false, typeInit, infos);
}


static Tokens getNextToken() { 
  return static_cast<Tokens>(benchlex());
}

///Defines the method for verifying the next token.
void assertNextToken(Tokens expectedToken) {
  if (getNextToken() != expectedToken) {
    switch (expectedToken) {
    case TOK_LP:
      CERR("left parenthesis");
      break;
    case TOK_RP:
      CERR("right patenthesis");
      break;
    case TOK_E:
      CERR("expected equal");
      break;
    default:
      break;
    }
  }
}

///Checking for undefined and repeated definitions.
void checker(std::map<std::string, SymbolInfo> &infos) {
  int counterDef = 0, counterOut = 0;
  for (auto it = infos.begin(); it != infos.end(); it++) {
    for (auto &i : it->second.uses) {
      counterDef += i.def && i.typeInit != TOK_OUTPUT;
      counterOut += i.typeInit == TOK_OUTPUT;
    }
    if (!counterDef) {
      VIEW();
      throw std::logic_error("unknow definition");
    }
    if (counterDef > 1) {
      VIEW();
      throw std::logic_error("repeated definitions");
    }
    if (counterOut > 1) {
      VIEW();      
      throw std::logic_error("repeated definitions");
    }
    counterDef = 0;
    counterOut = 0;
  }
}

///Method for verifying the next id and filling in the symbol table.
std::string assertNextId(Tokens token,
                         std::map<std::string, SymbolInfo> &infos) {
  if (getNextToken() != TOK_ID) {
    CERR("ID reading");
  }   
  std::string arg {benchtext};
  if (token == TOK_INPUT || token == TOK_OUTPUT) {
    addVarDef(token, infos);
  } else {
    addVarUse(token, infos);
  } 
  return arg;
}

//===----------------------------------------------------------------------===//
// checking the expression of the form and filling in the symbol table : 
//   ... (ID) 
//===----------------------------------------------------------------------===//

std::vector<std::string>
parseParenthesisInOut (Tokens token, std::map<std::string, SymbolInfo> &infos) {
  std::vector<std::string> name;
  assertNextToken(TOK_LP);
  name.push_back(assertNextId(token, infos));
  assertNextToken(TOK_RP);
  return name;
}

//===----------------------------------------------------------------------===//
// checking the expression of the form and filling in the symbol table : 
//   ... (ID, ID) ... 
//===----------------------------------------------------------------------===//

std::vector<std::string>
parseParenthesisID(Tokens token, std::map<std::string, SymbolInfo> &infos) {
  Tokens tok;
  std::vector<std::string> args;
  std::string arg;
  assertNextToken(TOK_LP);
  arg = assertNextId(token, infos);
  args.push_back(arg);
  while ((tok = getNextToken()) == TOK_COMMA) {
    arg = assertNextId(token, infos);
    args.push_back(arg);
  }
  if (tok != TOK_RP) {
    CERR("right patenthesis");
  }
  return args;
}

//===----------------------------------------------------------------------===//
// checking the expression of the form and filling in the symbol table : 
//   ... = AND/OR/NAND/NOR/DFF/NOT ... 
//===----------------------------------------------------------------------===//

void parseID(std::map<std::string, SymbolInfo> &infos) {
  auto &info = addVarDef(TOK_E, infos);
  assertNextToken(TOK_E);
  Tokens token = getNextToken();
  switch (token) { 
  case TOK_AND:
  case TOK_OR:
  case TOK_NAND:
  case TOK_NOR:
    info.uses.back().typeInit = token;
    info.args = parseParenthesisID(token, infos);
    break;
  case TOK_DFF:
  case TOK_NOT:
    info.uses.back().typeInit = token;
    info.args = parseParenthesisInOut(token, infos);
    break;
  default:
    CERR("expected function");
    break;
  }
}

///Method for building gnet.
std::unique_ptr<GNet> builderGnet(std::map<std::string, SymbolInfo> &infos) {
  std::unique_ptr<GNet> net = std::make_unique<GNet>();
  bool dffFlag = false;
  Gate::Id dffClock = -1;
  for (auto it = infos.begin(); it != infos.end(); it++) {
    for (const auto &i : it->second.uses) {
      if (i.typeInit == TOK_INPUT) {
        it->second.gateId = net->addIn();
      } else if (i.typeInit == TOK_DFF && !dffFlag) {
        it->second.gateId = net->newGate();
        dffClock = net->addIn();
        dffFlag = true;
        break;
      } else if (i.def){
        it->second.gateId = net->newGate();
        break;
      }
    }
  }
  for (auto it = infos.begin(); it != infos.end(); it++) {
    Tokens tok = TOK_INPUT;
    for (const auto &i : it->second.uses) {
      if (i.typeInit == TOK_OUTPUT) {
        net->addOut(it->second.gateId);
      }
    }
    for (const auto &i : it->second.uses) {
      if (i.def && tok < 3) {
        tok = i.typeInit;
      }
    }
    std::vector<Signal> argIds;
    for (const auto &i : it->second.args) {
      argIds.push_back(Signal::always(infos[i].gateId));
    }
    switch (tok) {
    case TOK_NOT:
      net->setGate(it->second.gateId, GateSymbol::NOT, argIds);
      break;
    case TOK_AND:
      net->setGate(it->second.gateId, GateSymbol::AND, argIds);
      break;
    case TOK_OR:
      net->setGate(it->second.gateId, GateSymbol::OR, argIds);
      break;
    case TOK_NAND:
      net->setGate(it->second.gateId, GateSymbol::NAND, argIds);
      break;
    case TOK_NOR:
      net->setGate(it->second.gateId, GateSymbol::NOR, argIds);
      break;
    case TOK_DFF:
      argIds.push_back(Signal::always(dffClock));
      net->setGate(it->second.gateId, GateSymbol::DFF, argIds);
      break;
    default:
      break;
    }
  }
  return net;
}

//===----------------------------------------------------------------------===//
// function calling checks, building gnet, checking the expression of the form: 
// INPUT/OUTPUT ...
// or
// ID ...  
//===----------------------------------------------------------------------===//

std::unique_ptr<GNet> parseBenchFile(const std::string &filename) {
  benchlineno = 1;
  benchin = fopen(filename.c_str(), "r");
  std::unique_ptr<GNet> ref = std::make_unique<GNet>();
  std::map<std::string, SymbolInfo> infos;
  if (!benchin) {
    std::cerr << std::endl << "unable to open file: ";
    throw std::ios_base::failure(filename);
  }
  try {
    while (Tokens token = getNextToken()) {
      switch (token) {
      case TOK_INPUT:
      case TOK_OUTPUT:
        parseParenthesisInOut(token, infos);
        break;
      case TOK_ID:
        parseID(infos);
        break;
      default:
        CERR("need input, output or id");
        break;
      }
    }
    checker(infos);
    ref = builderGnet(infos);
  } catch (std::exception& e) {
    fclose(benchin);
    std::cerr << "error in " << e.what() <<  std::endl;       
    throw;
  }
  fclose(benchin);
  return ref;
}
