#ifndef GLVERILOG_TOKEN_H
#define GLVERILOG_TOKEN_H
#define YY_DECL int scan_token() 
enum token_t
{
  EOF_TOKEN,
  MODULE,
  ENDMODULE,
  INPUT,
  OUTPUT,
  WIRE,
  REG,
  SEMICOLON,
  COLON,
  LBRACKET,
  RBRACKET,
  RBRACE,
  LBRACE,
  STRING,
  NUM,
  COMMA,
  EQUALS,
  ASSIGN,
  NOT,
  NAND,
  AND,
  XOR,
  NOR,
  LFIGURNAYA,
  RFIGURNAYA,
  START
  
};

    enum kind_of_error
{
  SUCCESS,
  FAILURE_IN_MODULE_NAME,
  FAILURE_IN_PARSE_NAME_LIST,
  FAILURE_IN_DECL,
  FAILURE_IN_MODULE_INCAPTULATION,
  FAILURE_IN_EXPR,
  FAILURE_IN_ARG,
  FAILURE_IN_ASSIGN,
  FAILURE_IN_GATE_LEVEL_VERILOG
};

    enum familyInfo
{
  VOID_,
  MODULE_,
  INPUT_,
  OUTPUT_,
  WIRE_,
  ASSIGN_,
  FUNCTION_,
  FUNC_INI_,
  LOGIC_GATE_
};
#endif
