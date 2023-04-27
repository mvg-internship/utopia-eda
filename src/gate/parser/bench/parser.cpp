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

#include <iostream>
#include <map>
#include <memory>
#include <stdlib.h>
#include <vector>

extern "C" int scan_token();

///Defines the compare method for TokenMap.name.
#define COMPARE(text, line, errorString, expectedString) do {\
  if (yytext == i.name) {\
    errorString = text;\
    expectedString.assign(yytext);\
  }\
} while (false)

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;

struct TokenMap {
  std::string name;
  int definite;
  Tokens typeInit;
  int usedInGate = 0;
  int nextNewLine = 0;
};

///Defines the adding in a token map vector.
void addMap(int definite, Tokens typeInit, std::vector<TokenMap> &maps) {
  char* text = new char[strlen(yytext)];
  strncpy(text, yytext, strlen(yytext) + 1);
  TokenMap map {text, definite, typeInit};
  maps.push_back(map);
}

Tokens getNextToken(std::size_t &place, std::size_t &line) { 
  Tokens val = static_cast<Tokens>(scan_token());
  // std::cout << " token: " << val << "\t" << yytext << "\tline: " 
  //   << line << " \tplace: " << place << std::endl;
  place += 1;
  return val;
}

///Defines the method for verifying the next token.
void assertNextToken(Tokens expectedToken,
                     const char* _errorString,
                     size_t &place,
                     size_t &line,
                     std::string &errorString,
                     std::string &expectedString) {
  Tokens token = getNextToken(place, line);
  if (token != expectedToken) {
    errorString = _errorString;
    expectedString.assign(yytext);
  }
}

void unknown(std::vector<TokenMap> &maps,
             std::string &errorString,
             int &where,
             std::string &unknownToken,
             int &n) {
  int flag = 0;
  where = n + 1;
  int counter = 0;
  for (auto i : maps) {
    for (auto j : maps) {
      if (i.name == j.name) {
        counter += 1;
      }
      if ((i.definite == 0 && i.name == j.name && j.definite == 1)
           || i.definite == 1) {
        flag = 1;
      }
    }
    if (flag == 0 || (counter == 1 && i.definite == 0)) {
      unknownToken = i.name;
      errorString = "ERROR IN UNKNOWN DEFINITION ";
      break;
    }
    if (i.definite == 1) {
      where += 1;
    }
    if (i.nextNewLine == 1) {
      where += 1;
    }
    counter = 0;
  }
}

void output(Tokens type,
            std::size_t &line,
            std::vector<TokenMap> &maps,
            std::string &errorString,
            std::string &expectedString) {
  for (auto i : maps) {
    if (type != TOK_OUTPUT) {
      if (strlen(yytext) == i.name.size() && i.typeInit == TOK_OUTPUT)
        COMPARE("ENTRY OUTPUT", line, errorString, expectedString);
    }
  }
}

void doubleDefinition(Tokens whereCalled,
                      std::size_t &line,
                      std::vector<TokenMap> &maps,
                      std::string &errorString,
                      std::string &expectedString) {
  for (auto i : maps) {
    if (i.definite == 1) { // definite = {0 if not declared, 1 if declared}.
      if (strlen(yytext) == i.name.size() && whereCalled != TOK_OUTPUT
          && i.typeInit != TOK_OUTPUT)  {
        COMPARE("DOUBLE DEFINITION", line, errorString, expectedString);
      } else if (strlen(yytext) == i.name.size() 
                 && whereCalled == TOK_OUTPUT) {
          if(i.typeInit == TOK_OUTPUT)
            COMPARE("DOUBLE DEFINITION", line, errorString, expectedString);
      }
    }
  }
}

void assertNextId(Tokens expectedToken,
                  const char* _errorString, 
                  Tokens whereCalled,
                  std::size_t &place,
                  std::size_t &line,
                  std::vector<TokenMap> &maps,
                  std::string &errorString, 
                  std::string &expectedString) {
  Tokens token = getNextToken(place, line); 
  if (token != expectedToken) {
    errorString = _errorString;
    expectedString.assign(yytext);
  }   
  if (whereCalled == TOK_INPUT && errorString.size() == 0) { // checkers.
    doubleDefinition(TOK_INPUT, line, maps, errorString, expectedString);
    addMap(1, TOK_INPUT, maps);
  } else if (whereCalled == TOK_OUTPUT && errorString.size() == 0) {
    doubleDefinition(TOK_OUTPUT, line, maps, errorString, expectedString);
    addMap(1, TOK_OUTPUT, maps);
  } else if (errorString.size() == 0) {
    output(whereCalled, line, maps, errorString, expectedString);
    addMap(0, whereCalled, maps);
  } 
}

void parseParenthesisInOut(Tokens type, 
                        std::size_t &place,
                        std::size_t &line,
                        std::vector<TokenMap> &maps,
                        std::string &errorString,
                        std::string &expectedString) { 
  if (errorString.size() == 0) {
    assertNextToken(TOK_LP, "LP", place, line, errorString, expectedString);
  }
  if (errorString.size() == 0) {
    assertNextId(TOK_ID,
                "ID",
                type,
                place,
                line,
                maps,
                errorString,
                expectedString);
  }  
  if (errorString.size() == 0) {
    assertNextToken(TOK_RP, "RP", place, line, errorString, expectedString);
    if (errorString.size() == 0) {
      line += 1;
      place = 0;
    }
  }
}

void parseParenthesisID(Tokens type,
                        std::size_t &place,
                        std::size_t &line,
                        std::vector<TokenMap> &maps,
                        std::string &errorString,
                        std::string &expectedString) {
  Tokens token;
  if (errorString.size() == 0) {
    assertNextToken(TOK_LP,
                      "LP",
                      place,
                      line,
                      errorString,
                      expectedString);
  }
  if (errorString.size() == 0) {
    assertNextId(TOK_ID,
                "ID",
                type,
                place,
                line,
                maps,
                errorString,
                expectedString);
  }
  while ((token = getNextToken(place, line)) == TOK_COMMA) {
    if (errorString.size() == 0) {
      assertNextId(TOK_ID,
                  "ID",
                  type,
                  place,
                  line,
                  maps,
                  errorString,
                  expectedString);
    }
  }
  if (token != TOK_RP && errorString.size() == 0) {
    errorString = "'RP'";
    expectedString.assign(yytext);
  }
  if (errorString.size() == 0) {
    line += 1;
    place = 0;
  }
}

void parseID(std::size_t &place,
             std::size_t &line,
             std::vector<TokenMap> &maps,
             std::string &errorString, 
             std::string &expectedString) {
  addMap(1, TOK_E, maps); // adding.
  assertNextToken(TOK_E, "E", place, line, errorString, expectedString);
  Tokens token = getNextToken(place, line);
  if ((token == TOK_AND || token == TOK_OR ||
       token == TOK_NAND || token == TOK_NOR) && errorString.size() == 0) {
    maps.back().typeInit = token;
    parseParenthesisID(token, place, line, maps, errorString, expectedString);
  } else if ((token == TOK_DFF || token == TOK_NOT)
              && errorString.size() == 0) { 
    maps.back().typeInit = token;
    parseParenthesisInOut(token,
                          place,
                          line,
                          maps,
                          errorString,
                          expectedString);
  } else {
    errorString = "function";
    expectedString.assign(yytext);
  }
}

std::unique_ptr<GNet> builderGnet(std::vector<TokenMap> &maps) {
  std::unique_ptr<GNet> net = std::make_unique<GNet>();
  int dffFlag = 0;
  Gate::Id dffClock = -1;
  std::map<std::string, Gate::Id> gmap;
  for (auto i : maps) {
    if (i.usedInGate == 0 && gmap.find(i.name) == gmap.end()) {
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

  for (auto i : maps) {
    if (i.typeInit == TOK_OUTPUT) {
      net->addOut(gmap[i.name]);
    }
  }
  return net;
}

std::unique_ptr<GNet> parseBenchFile(const std::string &filename) {
  std::string errorString = "";
  std::string expectedString;
  std::size_t place = 0;
  std::size_t line = 1;
  std::vector<TokenMap> maps;
  int n = 0;
  
  yyin = fopen(filename.c_str(), "r");
  while (Tokens token = getNextToken(place, line)) {
    if (errorString.size() != 0)
      break;
    if (token == TOK_COMMENT)
      n += 1;
    if (token == TOK_N || token == TOK_COMMENT) {
      if (!maps.empty()) {
        maps.back().nextNewLine = 1;
      }
      line += 1;
      continue;
    }
    if (token == TOK_INPUT || token == TOK_OUTPUT) {
      parseParenthesisInOut(token,
                            place,
                            line,
                            maps,
                            errorString,
                            expectedString);
    } else if (token == TOK_ID && errorString.size() == 0) {
      if (errorString.size() == 0) {
        doubleDefinition(TOK_ID, line, maps, errorString, expectedString);
      }
      if (errorString.size() == 0) {
        parseID(place, line, maps, errorString, expectedString);
      }
    } else {
      errorString = "INPUT', 'OUTPUT' or 'ID";
    }
  }
  int where = 0;
  std::string unknownToken;
  if (errorString.size() == 0) {
    unknown(maps, errorString, where, unknownToken, n);
  }
  std::unique_ptr<GNet> ref = std::make_unique<GNet>();
  if (errorString.size() == 0) {
    ref = builderGnet(maps);
  }
  if (errorString == "ERROR IN UNKNOWN DEFINITION ") {
    std::cout << "error with unknown '" << unknownToken << "'\tpos: " << where << "\n";
  } else if (errorString.size() != 0) {
    std::cout << "error in '" << errorString << "'\tpos: "
    << line + 1 << ":" << place - 2 << "\tcaught " << expectedString << "\n";
  }
  fclose(yyin);
  std::cout << "\nthe file was read: " << filename << ".\n\n";
  return ref;
}

int main(int argc, char* argv[]) {

  for (int i = 1; i < argc; i++) {
    std::cout << *parseBenchFile(argv[i]);
    std::cout << "\nend of files\n";
  }
    return 0;
}
