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
#include "headerFile"
#include "tokens.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

extern "C" int scan_token();

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

struct TokenMap {
  const bool def;
  Tokens typeInit;  
  int line;
};

struct SymbolInfo {
  std::vector<TokenMap> uses;
  Gate::Id gateId;
  std::vector<std::string> args;
};

///Defines the adding in a token map vector.
std::string addMap(bool def, Tokens typeInit, std::map<std::string, SymbolInfo> &infos) {
  const std::string name {benchtext};
  auto &inf = infos[name];
  const TokenMap map {def, typeInit, benchlineno};
  inf.uses.push_back(map);
  return name;
}

Tokens getNextToken() { 
  return static_cast<Tokens>(scan_token());
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

void checker(std::map<std::string, SymbolInfo> &infos) {
  //checking for undefined definitions.
  for (auto it = infos.begin(); it != infos.end(); it++) {
    int count = std::count_if(it->second.uses.begin(), it->second.uses.end(),
                      [](const TokenMap& s) { return s.def == true && s.typeInit != TOK_OUTPUT; });
    if (count == 0) {
      std::cerr << "exeption token: " << it->first << " ";
      VIEW();
      throw std::logic_error("unknow definition");
    }
    if (count > 1) {
      VIEW();
      throw std::logic_error("repeated definitions");
    }
    count = std::count_if(it->second.uses.begin(), it->second.uses.end(),
                      [](const TokenMap& s) { return s.typeInit == TOK_OUTPUT; });
    if (count > 1) {
      VIEW();      
      throw std::logic_error("repeated definitions");
    }
  }
}

std::string assertNextId(Tokens token, std::map<std::string, SymbolInfo> &infos) {
  if (getNextToken() != TOK_ID) {
    CERR("ID reading");
  }   
  std::string arg {benchtext};
  if (token == TOK_INPUT || token == TOK_OUTPUT) {
    addMap(1, token, infos);
  } else {
    addMap(0, token, infos);
  } 
  return arg;
}
std::vector<std::string> parseParenthesisInOut(Tokens token, std::map<std::string, SymbolInfo> &infos) {
  std::vector<std::string> name;
  assertNextToken(TOK_LP);
  name.push_back(assertNextId(token, infos));
  assertNextToken(TOK_RP);
  return name;
}

std::vector<std::string> parseParenthesisID(Tokens token, std::map<std::string, SymbolInfo> &infos) {
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

void parseID(std::map<std::string, SymbolInfo> &infos) {
  std::string name = addMap(1, TOK_E, infos); // adding.
  assertNextToken(TOK_E);
  auto &info = infos[name];
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

std::unique_ptr<GNet> builderGnet(std::map<std::string, SymbolInfo> &infos) {
  std::unique_ptr<GNet> net = std::make_unique<GNet>();
  bool dffFlag = false;
  Gate::Id dffClock = -1;
  for (auto it = infos.begin(); it != infos.end(); it ++) {
    for (const auto &i : it->second.uses) {
      if (i.typeInit == TOK_INPUT) {
          it->second.gateId = net->addIn();
      } else if (i.typeInit == TOK_DFF && dffFlag == false) {
          dffClock = net->addIn();
          dffFlag = true;
      } else if (i.def == true){
          it->second.gateId = net->newGate();
        break;
      }
    }
  }
  for (auto it = infos.begin(); it != infos.end(); it++) {
    Tokens tok;
    for (const auto &i : it->second.uses) {
      if (i.typeInit == TOK_OUTPUT) {
        net->addOut(it->second.gateId);
      }
    }
    for (const auto &i : it->second.uses) {
      if (i.def == true) {
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
      net->setGate(it->second.gateId, GateSymbol::DFF, argIds );
      break;
    default:
      break;
    }
  }
  return net;
}

std::unique_ptr<GNet> parseBenchFile(const std::string &filename) {
  benchlineno = 1;
  benchin = fopen(filename.c_str(), "r");
  std::unique_ptr<GNet> ref = std::make_unique<GNet>();
    std::vector<TokenMap> maps;
    std::map<std::string, SymbolInfo> infos;
    if (!benchin) {
      std::cerr << std::endl << "unable to open file: ";
      throw std::ios_base::failure(filename);
    }
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
    fclose(benchin);
  return ref;
}

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    try {
        std::cout << *parseBenchFile(argv[i]);
    } catch (std::exception& e) {
        std::cerr << "error in " << e.what() <<  std::endl; 
    }
  }
  return 0;
}
