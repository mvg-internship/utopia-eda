//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include <FlexLexer.h>
#include "lexer.c"

#include <iostream>
#include <stdlib.h>
#include <vector>

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
#define ALERT(error_string) do {\
  std::cout << " expected '" << error_string << "'\tline: "\
    << line << "\tСaught " << yytext << std::endl;\
  exit(EXIT_FAILURE);\
} while (false) 

///Defines the method for verifying the next token.
#define ASSERT_NEXT_TOKEN(expected_token, error_string) do {\
  Tokens token = getNextToken();\
  if ( token != expected_token ) {\
    ALERT(error_string);\
  }\
} while (false)

///Defines the adding in a token map vector.
#define MAPS(definite, type_init) do {\
  char* text = new char[strlen(yytext)];\
  strcpy(text, yytext);\
  TokenMap map {text, definite, type_init};\
  maps.push_back(map);\
} while (false)

struct TokenMap {
  const char* name;
  int definite; // 0 = NOT init, 1 = init
  int type_init; // 0 = input, 1 = output, 2 = function
};

std::vector<TokenMap> maps;

static std::size_t place = 0;
static std::size_t line = 0;

void unknown() {
  std::vector<TokenMap> maps_upd;
  for (auto& i : maps) {
    for (auto& j : maps) {
      int k = 0; 
      if (i.definite == 0 && strlen(j.name) 
        == strlen(i.name) && j.definite == 1) {
        for (int z = 0; z < strlen(j.name); z++) {
          if (i.name[z] == j.name[z])
            k += 1;
        }
        if (k == strlen(i.name)) {
          i.definite = 1;
          maps_upd.push_back(i);
        }
      }
    }
  }
  for (auto& i : maps) {
    if(i.definite == 1)
    maps_upd.push_back(i);
  }
  for (auto& i : maps_upd) {
    if (size(maps_upd) != size(maps)) {
      std::cout << "ERROR IN UNKNOWN DEFINITION " << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

void input() {
  for (auto& i : maps) {
    int k = 0;
    if (strlen(yytext) == strlen(i.name) && i.type_init == 0)
      COMPARE("ENTRY INPUT");
  }
} 

void output(Tokens type) {
  for (auto& i : maps) {
    if (type != OUTPUT) {
      int k = 0;
      if (strlen(yytext) == strlen(i.name) && i.type_init == 1)
        COMPARE("ENTRY OUTPUT");
    }
  }
}

void doubleDefinition(Tokens where_called) {
  for (auto& i : maps) {
    if (i.definite == 1) { // definite = {0 if not declared, 1 if declared}
      int k = 0;
      if (strlen(yytext) == strlen(i.name) && where_called != OUTPUT
        && i.type_init != 1)  {
        COMPARE("DOUBLE DEFINITION");
        } else if (strlen(yytext) == strlen(i.name) 
          && where_called == OUTPUT ) {
          if(i.type_init == 1)
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

void assertNextId(Tokens expected_token,
                    char* error_string, 
                    Tokens where_called) {
  Tokens token = getNextToken(); 
  if (token != expected_token) { 
    ALERT(error_string); 
  }   
  if (where_called == INPUT) {
    doubleDefinition(INPUT);
    MAPS(1, 0);
  } else if (where_called == OUTPUT) {
    doubleDefinition(OUTPUT);
    MAPS(1, 1);
  } else {
    output(where_called);
    MAPS(0, 2);
  } 
}

void parseParenthesisIO(Tokens type) { // type = {INPUT, OUTPUT, DFF, NOT}
  ASSERT_NEXT_TOKEN(LP, "LP");
  assertNextId(ID, "ID", type);
  ASSERT_NEXT_TOKEN(RP, "RP");
  line += 1;
}

void parseParenthesisID(Tokens type) { // type = {AND, OR, NAND, NOR}
  Tokens token;
  ASSERT_NEXT_TOKEN(LP, "LP");
  assertNextId(ID, "ID", type);
  while ((token = getNextToken()) == COMMA)
    assertNextId(ID, "ID", type);
  if (token != RP) {
    ALERT("'RP'");
  }
  line += 1;
}

void parseID() {
  ASSERT_NEXT_TOKEN(E, "E");
  Tokens token = getNextToken();
  if (token == AND || token == OR || token == NAND || token == NOR) {
    return parseParenthesisID(token);
  } else if (token == DFF || token == NOT) { 
    return parseParenthesisIO(token);
  } else {
    ALERT("function");
  }
}

void parseBenchFile() {
  while (Tokens token = getNextToken()) {
    if (token == INPUT || token == OUTPUT) {
      parseParenthesisIO(token);    // deepening to a level below 
    } else if (token == ID) {
      input();    // checker
      doubleDefinition(ID); // checker
      MAPS(1, 2); // adding
      parseID(); // deepening to a level below
    } else {
      ALERT("INPUT', 'OUTPUT' or 'ID"); 
    }
  }
  unknown(); // checker 
  std::cout << "END OF FILE.\n";
}

int main(int argc, char* argv[]) {

  for (int i = 1; i < argc; i++) {
    yyin = fopen( argv[i], "r" );
    parseBenchFile();
    std::cout << " end check\n\n";
    fclose(yyin);
  }
    return 0;
}
