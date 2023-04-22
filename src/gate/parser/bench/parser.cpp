//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <FlexLexer.h>
#include "gate/model/gnet.h"
#include "gate/model/gate.h"
#include "gate/model/gsymbol.h"
#include "headerFile"
#include "tokens.h"

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>

extern "C" int scan_token();

///Defines the compare method for TokenMap.name.
#define COMPARE(text) do {\
  for (int j = 0; j < strlen(yytext); j++) {\
    if (i.name[j] == yytext[j])\
      k += 1;\
  }\
  if (k == strlen(yytext)) {\
    std::cout << "ERROR IN " << text << " WITH "\
      << yytext << "\tline: " << line << std::endl;\
    exit(EXIT_FAILURE);\
  }\
} while (false)

///Defines the alert method for debugging information.
#define ALERT(errorString) do {\
  std::cout << " expected '" << errorString << "'\tline: "\
    << line << "\tСaught " << yytext << std::endl;\
  exit(EXIT_FAILURE);\
} while (false) 

///Defines the method for verifying the next token.
#define ASSERT_NEXT_TOKEN(expectedToken, errorString) do {\
  Tokens token = getNextToken();\
  if ( token != expectedToken ) {\
    ALERT(errorString);\
  }\
} while (false)

///Defines the adding in a token map vector.
#define MAPS(definite, typeInit) do {\
  char* text = new char[strlen(yytext)];\
  strcpy(text, yytext);\
  TokenMap map {text, definite, typeInit};\
  maps.push_back(map);\
} while (false)

using GNet = eda::gate::model::GNet;
using Signal = eda::gate::model::Gate::Signal;
using Gate = eda::gate::model::Gate;
using GateSymbol = eda::gate::model::GateSymbol;


struct TokenMap {
  const char* name;
  int definite; // 0 = NOT init, 1 = init
  Tokens typeInit; // 0 = input, 1 = output, 2 = function
  int usedInGate = 0;
  // Gate::Id gateId;
};

std::vector<TokenMap> maps;
static std::size_t place = 0;
static std::size_t line = 0;
GNet net;

void unknown() {
  std::vector<TokenMap> mapsUpd;
  int k = 0;
  for (auto i : maps) {
    for (auto j : maps) {
      k += 1;
      if (i.definite == 0 && j.definite == 1 &&
            static_cast<std::string>(i.name) == static_cast<std::string>(j.name)) {
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

void input() {
  for (auto i : maps) {
    int k = 0;
    if (strlen(yytext) == strlen(i.name) && i.typeInit == 1)
      COMPARE("ENTRY INPUT");
  }
} 

void output(Tokens type) {
  for (auto i : maps) {
    if (type != TOK_OUTPUT) {
      int k = 0;
      if (strlen(yytext) == strlen(i.name) && i.typeInit == 2)
        COMPARE("ENTRY OUTPUT");
    }
  }
}

void doubleDefinition(Tokens whereCalled) {
  for (auto i : maps) {
    if (i.definite == 1) { // definite = {0 if not declared, 1 if declared}.
      int k = 0;
      if (strlen(yytext) == strlen(i.name) && whereCalled != TOK_OUTPUT
        && i.typeInit != 2)  {
        COMPARE("DOUBLE DEFINITION");
        } else if (strlen(yytext) == strlen(i.name) 
          && whereCalled == TOK_OUTPUT ) {
          if(i.typeInit == 2)
            COMPARE("DOUBLE DEFINITION");
        }
    }
  }
}

Tokens getNextToken() { 
  Tokens val = static_cast<Tokens>(scan_token());
  // std::cout << " token: " << val << "\t" << yytext << "\tline: " 
    // << line << " \tplace: " << place << std::endl;
  place += 1;
  return val;
}

void assertNextId(Tokens expectedToken,
                  char* errorString, 
                  Tokens whereCalled) {
  Tokens token = getNextToken(); 
  if (token != expectedToken) { 
    ALERT(errorString); 
  }   
  if (whereCalled == TOK_INPUT) { // checkers.
    doubleDefinition(TOK_INPUT);
    MAPS(1, TOK_INPUT);
    // Gate::Id inOutId = net.addIn();
  } else if (whereCalled == TOK_OUTPUT) {
    doubleDefinition(TOK_OUTPUT);
    MAPS(1, TOK_OUTPUT);
  } else {
    output(whereCalled);
    MAPS(0, whereCalled);
  } 
}

void parseParenthesisIO(Tokens type) { // type = {INPUT, OUTPUT, DFF, NOT}
  ASSERT_NEXT_TOKEN(TOK_LP, "LP");
  assertNextId(TOK_ID, "ID", type);
  ASSERT_NEXT_TOKEN(TOK_RP, "RP");
  line += 1;
}

void parseParenthesisID(Tokens type) { // type = {AND, OR, NAND, NOR}
  Tokens token;
  ASSERT_NEXT_TOKEN(TOK_LP, "LP");
  assertNextId(TOK_ID, "ID", type);
  while ((token = getNextToken()) == TOK_COMMA)
    assertNextId(TOK_ID, "ID", type);
  if (token != TOK_RP) {
    ALERT("'RP'");
  }
  line += 1;
}

void parseID() {
  MAPS(1, TOK_E); // adding.
  ASSERT_NEXT_TOKEN(TOK_E, "E");
  Tokens token = getNextToken();
  if (token == TOK_AND || token == TOK_OR || token == TOK_NAND || token == TOK_NOR) {
    maps.back().typeInit = token;
    return parseParenthesisID(token);
  } else if (token == TOK_DFF || token == TOK_NOT) { 
    maps.back().typeInit = token;
    return parseParenthesisIO(token);
  } else {
    ALERT("function");
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

void builderGnet() {
  int dffFlag = 0;
  Gate::Id dffClock = -1;

  std::map<std::string, Gate::Id> gmap;
  for (auto i : maps) {
    if (i.usedInGate == 0 && gmap.find(static_cast<std::string>(i.name)) == gmap.end()) {
      if (i.typeInit == 1) {
        gmap.insert(std::make_pair(static_cast<std::string>(i.name), net.addIn()));
      } else if (dffFlag == 0 && i.typeInit == 3) {
        dffClock = net.addIn();
        dffFlag = 1;
      } else {
        gmap.insert(std::make_pair(static_cast<std::string>(i.name), net.newGate()));
      }
    }
  }

  for (auto it = maps.begin(); it != maps.end(); it++) {
    if (it->typeInit != 1 && it->typeInit != 2 && it->definite == 1) {
      std::vector<Signal> ids;
      auto arg = gmap[static_cast<std::string>(it->name)];
      auto _type = it->typeInit;
      for (auto newIt = it + 1; newIt != maps.end() && newIt->definite == 0; newIt++) {
        auto fId = gmap.find(static_cast<std::string>(newIt->name));
        ids.push_back(Signal::always(fId->second));
        it = newIt;
      }
      if (_type == 4) {
        net.setGate(arg, GateSymbol::NOT, ids);
      } else if (_type == 5) {
        net.setGate(arg, GateSymbol::AND, ids);
      } else if (_type == 6) {
        net.setGate(arg, GateSymbol::OR, ids);
      } else if (_type == 7) {
        net.setGate(arg, GateSymbol::NAND, ids);
      } else if (_type == 8) {
        net.setGate(arg, GateSymbol::NOR, ids);
      } else if (_type == 3) {
        ids.push_back(Signal::always(dffClock));
        net.setGate(arg, GateSymbol::DFF, ids );
      } 
    }
  }

  for (auto i : maps) {
    if (i.typeInit == 2) {
      net.addOut(gmap[static_cast<std::string>(i.name)]);
    }
  }
  std::cout << net;
}

void parseBenchFile() {
  while (Tokens token = getNextToken()) {
    if (token == TOK_INPUT || token == TOK_OUTPUT) {
      parseParenthesisIO(token);    // deepening to a level below.
    } else if (token == TOK_ID) {
      input();    // checker.
      doubleDefinition(TOK_ID); // checker.
      parseID(); // deepening to a level below.
    } else {
      ALERT("INPUT', 'OUTPUT' or 'ID"); 
    }
  }
  unknown(); // checker for undeclared var.
  builderGnet();  
  std::cout << "END OF FILE.\n";
}

int main(int argc, char* argv[]) {

  std::cout<< net ;

  for (int i = 1; i < argc; i++) {
    yyin = fopen( argv[i], "r" );
    parseBenchFile();
    std::cout << "end check\n\n";
    fclose(yyin);
  }
    return 0;
}
