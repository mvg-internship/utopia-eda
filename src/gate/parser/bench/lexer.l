%{
#include "tokens.h"
%}

%option noyywrap 
%option prefix="bench"
IDD       [a-zA-Z_][a-zA-Z0-9_.]*

%%
[\t ]+
[\n]                                    { benchlineno++; } 
"#".*[\n]                               { benchlineno++; }

"DFF"                                   { return TOK_DFF; }
"NOT"                                   { return TOK_NOT; }
"AND"                                   { return TOK_AND; }
"OR"                                    { return TOK_OR; }
"NAND"                                  { return TOK_NAND; }
"NOR"                                   { return TOK_NOR; }
"INPUT"                                 { return TOK_INPUT; }
"OUTPUT"                                { return TOK_OUTPUT; }
"("                                     { return TOK_LP; }
")"                                     { return TOK_RP; } 
"="                                     { return TOK_E; }
","                                     { return TOK_COMMA; }

{IDD}                                    { return TOK_ID; }

%%
