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

#include <iostream>

extern "C" int scan_token();

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

void assertNextId() {
  if (getNextToken() != TOK_ID) {
    throw;
  }
}

void parseParenthesisInOut () {
  assertNextToken(TOK_LP);
  assertNextId(token, infos);
  assertNextToken(TOK_RP);
}

void parseParenthesisID() {
  Tokens tok;
  assertNextToken(TOK_LP);
  assertNextId();
  while ((tok = getNextToken()) == TOK_COMMA) {
    assertNextId();
  }
  if (tok != TOK_RP) {
    throw;
  }
}

void parseID() {
  assertNextToken(TOK_E);
  Tokens token = getNextToken();
  switch (token) { 
  case TOK_AND:
  case TOK_OR:
  case TOK_NAND:
  case TOK_NOR:
    parseParenthesisID();
    break;
  case TOK_DFF:
  case TOK_NOT:
    parseParenthesisInOut();
    break;
  default:
    throw;
    break;
  }
}

bool parseBenchFile(const std::string &filename) {
  benchlineno = 1;
  benchin = fopen(filename.c_str(), "r");
  try {
    if (!benchin) {
        throw;
    }
    while (Tokens token = getNextToken()) {
      switch (token) {
      case TOK_INPUT:
      case TOK_OUTPUT:
        parseParenthesisInOut(token);
        break;
      case TOK_ID:
        parseID();
        break;
      default:
        throw;
        break;
      }
    }
  } catch (...) {
    fclose(benchin);
    return false;
  }
  fclose(benchin);
  return true;
}
