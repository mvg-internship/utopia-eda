//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "headerFile"
#include "gate/parser/bench/tokens.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

extern "C" int scan_token();

struct VarInfo {
  const bool def;
  Tokens typeInit;  
  int line;
};

struct SymbolInfo {
  std::vector<VarInfo> uses;
  std::vector<std::string> args;
};

SymbolInfo& addVarInfo(bool def,
                       Tokens typeInit,
                       std::map<std::string, SymbolInfo> &infos) {
  const std::string name {benchtext};
  auto &inf = infos[name];
  const VarInfo map {def, typeInit, benchlineno};
  inf.uses.push_back(map);
  return inf;
}

Tokens getNextToken() { 
  return static_cast<Tokens>(scan_token());
}

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
  int counter1 = 0, counter2 = 0;
  for (auto it = infos.begin(); it != infos.end(); it++) {
    for (auto &i : it->second.uses) {
      counter1 += i.def && i.typeInit != TOK_OUTPUT;
      counter2 += i.typeInit == TOK_OUTPUT;
    }
    if (!counter1) {
      VIEW();
      throw std::logic_error("unknow definition");
    }
    if (counter1 > 1) {
      VIEW();
      throw std::logic_error("repeated definitions");
    }
    if (counter2 > 1) {
      VIEW();      
      throw std::logic_error("repeated definitions");
    }
    counter1 = 0;
    counter2 = 0;
  }
}

std::string assertNextId(Tokens token,
                         std::map<std::string, SymbolInfo> &infos) {
  if (getNextToken() != TOK_ID) {
    CERR("ID reading");
  }   
  std::string arg {benchtext};
  if (token == TOK_INPUT || token == TOK_OUTPUT) {
    addVarInfo(1, token, infos);
  } else {
    addVarInfo(0, token, infos);
  } 
  return arg;
}

std::vector<std::string>
parseParenthesisInOut (Tokens token, std::map<std::string, SymbolInfo> &infos) {
  std::vector<std::string> name;
  assertNextToken(TOK_LP);
  name.push_back(assertNextId(token, infos));
  assertNextToken(TOK_RP);
  return name;
}

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

void parseID(std::map<std::string, SymbolInfo> &infos) {
  auto &info = addVarInfo(1, TOK_E, infos);
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

bool parseBenchFile(const std::string &filename) {
  benchlineno = 1;
  benchin = fopen(filename.c_str(), "r");
  std::unique_ptr<GNet> ref = std::make_unique<GNet>();
  std::vector<VarInfo> maps;
  std::map<std::string, SymbolInfo> infos;
  try {
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
  } catch (std::exception& e) {
    fclose(benchin);
    std::cerr << "error in " << e.what() <<  std::endl; 
    return false;      
  }
  fclose(benchin);
  return ref;
}
