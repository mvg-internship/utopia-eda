%{
#include <stdio.h>
#define YY_DECL int scan_token()
enum Tokens {
    TOK_INPUT = 1,
    TOK_OUTPUT,
    TOK_DFF,
    TOK_NOT,
    TOK_AND,
    TOK_OR,
    TOK_NAND,
    TOK_NOR,
    TOK_LP, //left parenthesis
    TOK_RP, //right paranthesis
    TOK_E,  // equal
    TOK_ID, //var
    TOK_COMMA,
};
%}
%x IN_COMMENT
%option noyywrap 
IDD       [a-zA-Z][a-zA-Z0-9]*

%%

[ \t\r\n]+

<INITIAL>{
"#"              BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
\n      BEGIN(INITIAL);
[a-zA-Z][a-zA-Z0-9]*
\n
}

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
