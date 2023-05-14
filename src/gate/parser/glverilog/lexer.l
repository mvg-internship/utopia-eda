%{
        //Define section
        #include <stdio.h>
        #include <gate/parser/glverilog/token.h>
        
%}

%option noyywrap yylineno
%x IN_COMMENT
 
NUM     [0-9]
STRING  [a-zA-Z][a-zA-Z0-9_]*

        //Rule section
%%

\n

<INITIAL>{
"/*"              BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
"*/"      BEGIN(INITIAL);
[^*\n]+   // eat comment in chunks
"*"       // eat the lone star
\n
}

module      { return MODULE; }
endmodule   { return ENDMODULE; }

input   { return INPUT; }
output  { return OUTPUT; }   
wire    { return WIRE; }
reg     { return REG; }
assign  { return ASSIGN; }
not     { return NOT; }
nand    { return NAND; }
and     { return AND; }
xor     { return XOR; }
nor     { return NOR; }
or      { return OR; }
xnor    { return XNOR; }
dff     { return DFF; }



";"     { return SEMICOLON; }
":"     { return COLON; }
"("     { return LBRACE; }
")"     { return RBRACE; }
"["     { return LBRACKET; }
"]"     { return RBRACKET; }
","     { return COMMA; }
"="     { return EQUALS; }
"{"     { return LFIGURNAYA; }
"}"     { return RFIGURNAYA; }


{STRING}    { return STRING; }
{NUM} { return NUM; }
