//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <FlexLexer.h>
#include <iostream>
#include <map>
#include <memory>
#include <stdlib.h>
#include <vector>

#include "gate/model/gate.h"
#include "gate/model/gnet.h"
#include "gate/model/gsymbol.h"
#include "headerFile"
#include "tokens.h"

extern "C" int scan_token();

///Defines the compare method for TokenMap.name.
#define COMPARE(text, line) do {\
  if (yytext == i.name) {\
    std::cout << "ERROR IN " << static_cast<const char*>(text) << " WITH "\
      << yytext << "\tline: " << line << std::endl;\
    exit(EXIT_FAILURE);\
  }\
} while (false)

///Defines the alert method for debugging information.
#define ALERT(errorString, line) do {\
  std::cout << " expected '" << errorString << "'\tline: "\
    << line << "\tСaught " << yytext << std::endl;\
  exit(EXIT_FAILURE);\
} while (false) 

///Defines the method for verifying the next token.
#define ASSERT_NEXT_TOKEN(expectedToken, errorString, place, line) do {\
  Tokens token = getNextToken(place, line);\
  if ( token != expectedToken ) {\
    ALERT(errorString, line);\
  }\
} while (false)

///Defines the adding in a token map vector.
#define MAPS(definite, typeInit, maps) do {\
  char* text = new char[strlen(yytext)];\
  strncpy(text, yytext, strlen(yytext) + 1);\
  TokenMap map {text, definite, typeInit};\
  maps.push_back(map);\
} while (false)

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;

struct TokenMap {
  std::string name;
  int definite; // 0 = NOT init, 1 = init
  Tokens typeInit; // 0 = input, 1 = output, 2 = function
  int usedInGate = 0;
  // Gate::Id gateId;
};

void unknown(std::vector<TokenMap>& maps) {
  std::vector<TokenMap> mapsUpd;
  for (auto i : maps) {
    for (auto j : maps) {
      if (i.definite == 0 && j.definite == 1 && i.name == j.name) {
          i.definite = 1;
          mapsUpd.push_back(i);
      }
    }
  }
  for (auto i : maps) {
    if(i.definite == 1)
    mapsUpd.push_back(i);
  }
  for (auto i : mapsUpd) {
    if (size(mapsUpd) != size(maps)) {
      std::cout << "ERROR IN UNKNOWN DEFINITION " << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

void input(std::size_t &line, std::vector<TokenMap>& maps) {
  for (auto i : maps) {
    if (strlen(yytext) == i.name.size() && i.typeInit == 1)
      COMPARE("ENTRY INPUT", line);
  }
} 

void output(Tokens type, std::size_t &line, std::vector<TokenMap>& maps) {
  for (auto i : maps) {
    if (type != TOK_OUTPUT) {
      if (strlen(yytext) == i.name.size() && i.typeInit == 2)
        COMPARE("ENTRY OUTPUT", line);
    }
  }
}

void doubleDefinition(Tokens whereCalled, std::size_t &line, std::vector<TokenMap>& maps) {
  for (auto i : maps) {
    if (i.definite == 1) { // definite = {0 if not declared, 1 if declared}.
      if (strlen(yytext) == i.name.size() && whereCalled != TOK_OUTPUT
        && i.typeInit != 2)  {
        COMPARE("DOUBLE DEFINITION", line);
        } else if (strlen(yytext) == i.name.size() 
          && whereCalled == TOK_OUTPUT ) {
          if(i.typeInit == 2)
            COMPARE("DOUBLE DEFINITION", line);
        }
    }
  }
}

Tokens getNextToken(std::size_t &place, std::size_t &line) { 
  Tokens val = static_cast<Tokens>(scan_token());
  std::cout << " token: " << val << "\t" << yytext << "\tline: " 
    << line << " \tplace: " << place << std::endl;
  place += 1;
  return val;
}

void assertNextId(Tokens expectedToken,
                  const char* errorString, 
                  Tokens whereCalled,
                  std::size_t &place,
                  std::size_t &line,
                  std::vector<TokenMap> &maps) {
  Tokens token = getNextToken(place, line); 
  if (token != expectedToken) { 
    ALERT(errorString, line); 
  }   
  if (whereCalled == TOK_INPUT) { // checkers.
    doubleDefinition(TOK_INPUT, line, maps);
    MAPS(1, TOK_INPUT, maps);
    // Gate::Id inOutId = net.addIn();
  } else if (whereCalled == TOK_OUTPUT) {
    doubleDefinition(TOK_OUTPUT, line, maps);
    MAPS(1, TOK_OUTPUT, maps);
  } else {
    output(whereCalled, line, maps);
    MAPS(0, whereCalled, maps);
  } 
}

void parseParenthesisIO(Tokens type, 
                        std::size_t &place,
                        std::size_t &line,
                        std::vector<TokenMap> &maps) { // type = {INPUT, OUTPUT, DFF, NOT}
  ASSERT_NEXT_TOKEN(TOK_LP, "LP", place, line);
  assertNextId(TOK_ID, "ID", type, place, line, maps);
  ASSERT_NEXT_TOKEN(TOK_RP, "RP", place, line);
  line += 1;
}

void parseParenthesisID(Tokens type, std::size_t &place, std::size_t &line, std::vector<TokenMap> &maps) { // type = {AND, OR, NAND, NOR}
  Tokens token;
  ASSERT_NEXT_TOKEN(TOK_LP, "LP", place, line);
  assertNextId(TOK_ID, "ID", type, place, line, maps);
  while ((token = getNextToken(place, line)) == TOK_COMMA)
    assertNextId(TOK_ID, "ID", type, place, line, maps);
  if (token != TOK_RP) {
    ALERT("'RP'", line);
  }
  line += 1;
}

void parseID(std::size_t &place, std::size_t &line, std::vector<TokenMap>& maps) {
  MAPS(1, TOK_E, maps); // adding.
  ASSERT_NEXT_TOKEN(TOK_E, "E", place, line);
  Tokens token = getNextToken(place, line);
  if (token == TOK_AND || token == TOK_OR || token == TOK_NAND || token == TOK_NOR) {
    maps.back().typeInit = token;
    parseParenthesisID(token, place, line, maps);
  } else if (token == TOK_DFF || token == TOK_NOT) { 
    maps.back().typeInit = token;
    parseParenthesisIO(token, place, line, maps);
  } else {
    ALERT("function", line);
  }
}

//   struct TokenMap {
//   const char* name;
//   bool definite; // 0 = NOT init, 1 = init
  // Tokens typeInit; 
//     TOK_INPUT = 1,
//     TOK_OUTPUT, // 2
//     TOK_DFF, // 3
//     TOK_NOT, // 4
//     TOK_AND, // 5
//     TOK_OR, // 6
//     TOK_NAND, // 7
//     TOK_NOR, // 8
//   Gate::Id gateId;
// };

std::unique_ptr<GNet> builderGnet(std::vector<TokenMap>& maps) {
  std::unique_ptr<GNet> net = std::make_unique<GNet>();
  int dffFlag = 0;
  Gate::Id dffClock = -1;

  std::map<std::string, Gate::Id> gmap;
  for (auto i : maps) {
    if (i.usedInGate == 0 && gmap.find(i.name) == gmap.end()) {
      if (i.typeInit == 1) {
        gmap.insert(std::make_pair(i.name, net->addIn()));
      } else if (dffFlag == 0 && i.typeInit == 3) {
        dffClock = net->addIn();
        dffFlag = 1;
      } else {
        gmap.insert(std::make_pair(i.name, net->newGate()));
      }
    }
  }

  for (auto it = maps.begin(); it != maps.end(); it++) {
    if (it->typeInit != 1 && it->typeInit != 2 && it->definite == 1) {
      std::vector<Signal> ids;
      auto arg = gmap[it->name];
      auto _type = it->typeInit;
      for (auto newIt = it + 1; newIt != maps.end() && newIt->definite == 0; newIt++) {
        auto fId = gmap.find(newIt->name);
        ids.push_back(Signal::always(fId->second));
        it = newIt;
      }
      if (_type == 4) {
        net->setGate(arg, GateSymbol::NOT, ids);
      } else if (_type == 5) {
        net->setGate(arg, GateSymbol::AND, ids);
      } else if (_type == 6) {
        net->setGate(arg, GateSymbol::OR, ids);
      } else if (_type == 7) {
        net->setGate(arg, GateSymbol::NAND, ids);
      } else if (_type == 8) {
        net->setGate(arg, GateSymbol::NOR, ids);
      } else if (_type == 3) {
        ids.push_back(Signal::always(dffClock));
        net->setGate(arg, GateSymbol::DFF, ids );
      } 
    }
  }

  for (auto i : maps) {
    if (i.typeInit == 2) {
      net->addOut(gmap[i.name]);
    }
  }
  // std::cout << *net;
  return net;
}

std::unique_ptr<GNet> parseBenchFile(const std::string &filename) {
  std::size_t place = 0;
  std::size_t line = 0;
  std::vector<TokenMap> maps;
  yyin = fopen(filename.c_str(), "r");
  while (Tokens token = getNextToken(place, line)) {
    if (token == TOK_INPUT || token == TOK_OUTPUT) {
      parseParenthesisIO(token, place, line, maps);    // deepening to a level below.
    } else if (token == TOK_ID) {
      input(line, maps);    // checker.
      doubleDefinition(TOK_ID, line, maps); // checker.
      parseID(place, line, maps); // deepening to a level below.
    } else {
      ALERT("INPUT', 'OUTPUT' or 'ID", line); 
    }
  }
  unknown(maps); // checker for undeclared var.
  auto ref = builderGnet(maps);
  fclose(yyin);
  std::cout << "END OF FILE.\n";
  return ref;
}

int main(int argc, char* argv[]) {

  for (int i = 1; i < argc; i++) {
    std::cout << *parseBenchFile(argv[i]);
    std::cout << "end check\n\n";
  }
    return 0;
}
