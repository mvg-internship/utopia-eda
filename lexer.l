%{
#include <stdio.h>
#include "tokens.h"
#define YY_DECL int scan_token()
%}

%option noyywrap 
IDD       [a-zA-Z][a-zA-Z0-9]*

%%

[ \t\r\n]+

"DFF"                                   { return DFF; }
"NOT"                                   { return NOT; }
"AND"                                   { return AND; }
"OR"                                    { return OR; }
"NAND"                                  { return NAND; }
"NOR"                                   { return NOR; }
"INPUT"                                 { return INPUT; }
"OUTPUT"                                { return OUTPUT; }
"("                                     { return LP; }
")"                                     { return RP; } 
"="                                     { return E; }
","                                     { return COMMA; }

{IDD}                                    { return ID; }

%%