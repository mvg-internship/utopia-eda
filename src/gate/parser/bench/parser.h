//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#pragma once

#include "headerFile"
#include "tokens.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

// extern "C" int scan_token();

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

SymbolInfo& addVarDef(Tokens typeInit,
                      std::map<std::string, SymbolInfo> &infos) {
  return addVarInfo(true, typeInit, infos);
}

SymbolInfo& addVarUse(Tokens typeInit,
                      std::map<std::string, SymbolInfo> &infos) {
  return addVarInfo(false, typeInit, infos);
}


Tokens getNextToken() { 
  return static_cast<Tokens>(scan_token());
}

void assertNextToken(Tokens expectedToken) {
  if (getNextToken() != expectedToken) {
    switch (expectedToken) {
    case TOK_LP:
      throw;
      break;
    case TOK_RP:
      throw;
      break;
    case TOK_E:
      throw;
      break;
    default:
      break;
    }
  }
}

void checker(std::map<std::string, SymbolInfo> &infos) {
  int counterDef = 0, counterOut = 0;
  for (auto it = infos.begin(); it != infos.end(); it++) {
    for (auto &i : it->second.uses) {
      counterDef += i.def && i.typeInit != TOK_OUTPUT;
      counterOut += i.typeInit == TOK_OUTPUT;
    }
    if (!counterDef) {
      throw std::logic_error("unknow definition");
    }
    if (counterDef > 1) {
      throw std::logic_error("repeated definitions");
    }
    if (counterOut > 1) {
      throw std::logic_error("repeated definitions");
    }
    counterDef = 0;
    counterOut = 0;
  }
}

std::string assertNextId(Tokens token,
                         std::map<std::string, SymbolInfo> &infos) {
  if (getNextToken() != TOK_ID) {
    throw;
  }   
  std::string arg {benchtext};
  if (token == TOK_INPUT || token == TOK_OUTPUT) {
    addVarDef(token, infos);
  } else {
    addVarUse(token, infos);
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
    throw;
  }
  return args;
}

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
    throw;
    break;
  }
}

bool parseBenchFile(const std::string &filename) {
  benchlineno = 1;
  benchin = fopen(filename.c_str(), "r");
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
        throw;
        break;
      }
    }
    checker(infos);
  } catch (std::exception& e) {
    fclose(benchin);
    return false;
  }
  fclose(benchin);
  return true;
}
