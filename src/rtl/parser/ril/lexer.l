/*===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===*/

%option prefix="rr"
%option yylineno
%option noyywrap
%option nounput

%{
  #include <string>
  #include <rtl/parser/ril/parser.tab.hpp>
%}

NUM  [1-9][0-9]*
VAL  0b[0-1]+
TYPE [su]:[1-9][0-9]*
VAR  [_a-zA-Z][_a-zA-Z0-9]*

%%
"input"    { return TOK_INPUT;   }
"output"   { return TOK_OUTPUT;  }
"wire"     { return TOK_WIRE;    }
"reg"      { return TOK_REG;     }
"if"       { return TOK_IF;      }
"posedge"  { return TOK_POSEDGE; }
"negedge"  { return TOK_NEGEDGE; }
"level0"   { return TOK_LEVEL0;  }
"level1"   { return TOK_LEVEL1;  }
"@"        { return TOK_AT;      }
"*"        { return TOK_STAR;    }
"="        { return TOK_ASSIGN;  }
"~"        { return TOK_NOT;     }
"&"        { return TOK_AND;     }
"|"        { return TOK_OR;      }
"^"        { return TOK_XOR;     }
"+"        { return TOK_ADD;     }
"-"        { return TOK_SUB;     }
";"        { return TOK_SEMI;    }
"("        { return TOK_LBRACK;  }
")"        { return TOK_RBRACK;  }
"{"        { return TOK_LCURLY;  }
"}"        { return TOK_RCURLY;  }
{NUM}      {
             rrlval = new std::string(yytext);
             return TOK_NUM;
           }
{VAL}      {
             rrlval = new std::string(yytext);
             return TOK_VAL;
           }
{TYPE}     {
             rrlval = new std::string(yytext);
             return TOK_TYPE;
           }
{VAR}      {
             rrlval = new std::string(yytext);
             return TOK_VAR;
           }
[ \t\r\n]+ { /* Whitespaces */   }
.          { return TOK_OTHER;   }
%%

