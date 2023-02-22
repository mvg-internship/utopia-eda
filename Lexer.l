%{
        //Define section
        #include <iostream>
        #include "token.h"       
        #define YY_DECL int scan_token();
        #define TOKEN(X) return token:: X;
        extern std::string* pStr;
        
%}

%option c++
%option noyywrap
 
NUM     [0-9]
STRING  [a-zA-Z][a-zA-Z0-9]*

        //Rule section
%%

module      {TOKEN(MODULE)}
endmodule   {TOKEN(ENDMODULE)}

input   {TOKEN(INPUT)}
output  {TOKEN(OUTPUT)}   
wire    {TOKEN(WIRE)}
reg     {TOKEN(REG)}

";"     {TOKEN(SEMICOLON)}
":"     {TOKEN(COLON)}
"("     {TOKEN(LBRACE)}
")"     {TOKEN(RBRACE)}
"["     {TOKEN(LBRACKET)}
"]"     {TOKEN(RBRACKET)}
","     {TOKEN(COMMA)}

                        // "&"|"|"         {pStr = new std::string(yytext, yylen);TOKEN(LOGICAL_OPERATOR)}

                        // if|case|else    {pStr = new std::string(yytext, yylen);TOKEN(CONDITIONAL_OPERATOR)}

                        // "+"|"-"|"*"|"/"|"?"   {pStr = new std::string(yytext, yylen);TOKEN(AN_OPERATOR)} // Тут добавил фигурные скобки. Не знаю почему без них вообще работало

"{"[^{}\n]*"}"  // eat up one-line comments 
[ \t\n]+        // eat up whitespace 

{ID}    {pStr = std::string(yytext, yylen); TOKEN(AN_IDENTIFIER)}
{DIGIT} {pStr = std::string(yytext, yylen); TOKEN(AN_VARIABLE)}

.       {TOKEN(UNRECOGNIZED_CHARACTER)}

<<EOF>> {TOKEN(TOK_EOF)}
