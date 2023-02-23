%{
#include <stdio.h>
#include "tokens.h"
#define YY_DECL int scan_token()
// "DFF"                                  а { return DFF; }
// "NOT"                                   { return NOT; }
// "AND"                                   { return AND; }
// "OR"                                    { return OR; }
// "NAND"                                  { return NAND; }
// "NOR"                                   { return NOR; }
%}

%option noyywrap 
ID       [a-z][a-z0-9]*

%%

[ \t\r\n]+

"INPUT"                                 { return INPUT; }
"OUTPUT"                                { return OUTPUT; }
"("                                     { return LP; }
")"                                     { return RP; } 
"="                                     { return E; }
","                                     { return COMMA; }
"STRING"                                { return STRING;}

{ID}                                    { return ID; }

%%
