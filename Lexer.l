%{
        //Define section
        #include <stdio.h>
        #include "token.h"
        #define YY_DECL int scan_token()      
%}


%option noyywrap
 
NUM     [0-9]
STRING  [a-zA-Z][a-zA-Z0-9]*

        //Rule section
%%

[ \t\n]+    

module      { return MODULE; }
endmodule   { return ENDMODULE; }

input   { return INPUT; }
output  { return OUTPUT; }   
wire    { return WIRE; }
reg     { return REG; }

";"     { return SEMICOLON; }
":"     { return COLON; }
"("     { return LBRACE; }
")"     { return RBRACE; }
"["     { return LBRACKET; }
"]"     { return RBRACKET; }
","     { return COMMA; }

{STRING}    { return STRING; }
{NUM} { return NUM; }

