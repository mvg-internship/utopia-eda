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
#include <set>
#include <stdlib.h>
#include <vector>

extern "C" int scan_token();

///define useful cerr-throw message.
#define CERR(message) do {\
  std::cerr << std::endl << "Caught "\
    << yytext << " line "<< yylineno << std::endl;\
  throw std::logic_error(message);\
} while (false)

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;

struct TokenMap {
  const std::string name;
  const int definite;
  Tokens typeInit;
};

///Defines the adding in a token map vector.
void addMap(int definite, Tokens typeInit, std::vector<TokenMap> &maps) {
  const std::string name {yytext};
  const TokenMap map {name, definite, typeInit};
  maps.push_back(map);
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

void checker(std::vector<TokenMap> &maps) {
  std::string unknownToken;
  std::set<std::string> setDef, setUnDef, setOuts;
  std::vector<std::string> vecDef, outs;
  //checking for undefined definitions.
  for (auto &i : maps) {
    if (i.definite == 1) {
      setDef.insert(i.name);
      if (i.typeInit != TOK_OUTPUT) {
        vecDef.push_back(i.name);
      } else {
        outs.push_back(i.name);
        setOuts.insert(i.name);
      }
    } else {
      setUnDef.insert(i.name);
    }
  }
  std::set<std::string> differenceSet;
  std::set_difference(setUnDef.begin(), setUnDef.end(),
                      setDef.begin(), setDef.end(),
                      std::inserter(differenceSet, differenceSet.begin()));
  if (differenceSet.size() != 0) {
  std::cerr << std::endl << "caught ";
  for (auto it = differenceSet.begin(); it != differenceSet.end(); ++it) {
    std::cerr << *it << " ";
  }
    throw std::logic_error("unknow definition");
  }
  //checking for the use of repeated definitions.
  for (auto &i : setDef) {
    vecDef.erase(std::find(vecDef.begin(), vecDef.end(), i));
  }
  if (vecDef.size() != 0) {
        std::cerr << std::endl << "repeated definitions elem: ";
        for (auto &i : vecDef) {
            std::cerr << i << " ";
        }
        throw std::logic_error("repeated definitions");
  }
  for (auto &i : setOuts) {
    outs.erase(std::find(outs.begin(), outs.end(), i));
  }
  if (outs.size() != 0) {
      std::cerr << std::endl << "repeated definitions elem: ";
      for (auto &i : outs) {
          std::cerr << i << " ";
      }
      throw std::logic_error("repeated definitions");
} 
  //checking for using OUTPUT as an input signal.
  std::set<std::string> outName;
  for (auto &i : maps) {
    if (i.typeInit == TOK_OUTPUT) {
      outName.insert(i.name);
    }
  }
  for (auto &i : maps) {
    if (outName.count(i.name) == 1 && i.definite == 0) {
      std::cerr << std::endl << "caught " << i.name << std::endl;
      throw std::logic_error("entry output token");
    }
  }
}

void assertNextId(Tokens token, std::vector<TokenMap> &maps) {
  if (getNextToken() != TOK_ID) {
    CERR("ID reading");
  }   
  if (token == TOK_INPUT || token == TOK_OUTPUT) {
    addMap(1, token, maps);
  } else {
    addMap(0, token, maps);
  } 
}

void parseParenthesisInOut(Tokens token, std::vector<TokenMap> &maps) { 
  assertNextToken(TOK_LP);
  assertNextId(token, maps);
  assertNextToken(TOK_RP);
}

void parseParenthesisID(Tokens token, std::vector<TokenMap> &maps) {
  Tokens tok;
  assertNextToken(TOK_LP);
  assertNextId(token, maps);
  while ((tok = getNextToken()) == TOK_COMMA) {
      assertNextId(token, maps);
  }
  if (tok != TOK_RP) {
    CERR("right patenthesis");
  }
}

void parseID(std::vector<TokenMap> &maps) {
  addMap(1, TOK_E, maps); // adding.
  assertNextToken(TOK_E);
  Tokens token = getNextToken();
  switch (token) { 
    case TOK_AND:
    case TOK_OR:
    case TOK_NAND:
    case TOK_NOR:
      maps.back().typeInit = token;
      parseParenthesisID(token, maps);
      break;
    case TOK_DFF:
    case TOK_NOT:
      maps.back().typeInit = token;
      parseParenthesisInOut(token, maps);
      break;
    default:
      CERR("expected function");
      break;
  }
}

std::unique_ptr<GNet> builderGnet(std::vector<TokenMap> &maps) {
  std::unique_ptr<GNet> net = std::make_unique<GNet>();
  int dffFlag {0};
  Gate::Id dffClock = -1;
  std::map<std::string, Gate::Id> gmap;
  for (auto &i : maps) {
    if (gmap.find(i.name) == gmap.end()) {
      if (i.typeInit == TOK_INPUT) {
        gmap.insert(std::make_pair(i.name, net->addIn()));
      } else if (dffFlag == 0 && i.typeInit == TOK_DFF) {
        dffClock = net->addIn();
        dffFlag = 1;
      } else {
        gmap.insert(std::make_pair(i.name, net->newGate()));
      }
    }
  }

  for (auto it = maps.begin(); it != maps.end(); it++) {
    if (it->typeInit != TOK_INPUT && it->typeInit
        != TOK_OUTPUT && it->definite == 1) {
      std::vector<Signal> ids;
      auto arg = gmap[it->name];
      auto _type = it->typeInit;
      for (auto newIt = it + 1; newIt != maps.end()
           && newIt->definite == 0; newIt++) {
        auto fId = gmap.find(newIt->name);
        ids.push_back(Signal::always(fId->second));
        it = newIt;
      }
      switch (_type) {
      case TOK_NOT:
        net->setGate(arg, GateSymbol::NOT, ids);
        break;
      case TOK_AND:
        net->setGate(arg, GateSymbol::AND, ids);
        break;
      case TOK_OR:
        net->setGate(arg, GateSymbol::OR, ids);
        break;
      case TOK_NAND:
        net->setGate(arg, GateSymbol::NAND, ids);
        break;
      case TOK_NOR:
        net->setGate(arg, GateSymbol::NOR, ids);
        break;
      case TOK_DFF:
        ids.push_back(Signal::always(dffClock));
        net->setGate(arg, GateSymbol::DFF, ids );
        break;
      default:
        break;
      } 
    }
  }
  for (auto &i : maps) {
    if (i.typeInit == TOK_OUTPUT) {
      net->addOut(gmap[i.name]);
    }
  }
  return net;
}

std::unique_ptr<GNet> parseBenchFile(const std::string &filename) {
  yylineno = 1;
  yyin = fopen(filename.c_str(), "r");
  std::unique_ptr<GNet> ref = std::make_unique<GNet>();
  try {
    std::vector<TokenMap> maps;
    if (!yyin) {
      std::cerr << std::endl << "unable to open file: ";
      throw std::ios_base::failure(filename);
    }
    while (Tokens token = getNextToken()) {
      switch (token) {
        case TOK_INPUT:
        case TOK_OUTPUT:
          parseParenthesisInOut(token, maps);
          break;
        case TOK_ID:
          parseID(maps);
          break;
        default:
          CERR("need input, output or id");
          break;
      }
    }
    checker(maps);
    ref = builderGnet(maps);
    fclose(yyin);
    std::cout << "\nthe file was read: " << filename << ".\n\n";
  } catch (std::exception& e) {
    std::cerr << "error in " << e.what() <<  std::endl; 
  }
  return ref;
}

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    std::cout << *parseBenchFile(argv[i]);
    std::cout << "\nend of files\n";
  }
  return 0;
}
