//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#ifndef GLVERILOG_TOKEN_H
#define GLVERILOG_TOKEN_H
#define YY_DECL int scan_token() 
enum Token_T
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
  OR,
  XNOR,
  DFF,
  LFIGURNAYA,
  RFIGURNAYA,
  START
  
};

    enum KindOfError
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

    enum FamilyInfo
{
  VOID_,
  MODULE_,
  INPUT_,
  OUTPUT_,
  WIRE_,
  ASSIGN_,
  FUNCTION_,
  FUNC_INI_,
  FUNC_INI_NAME_, 
  NOT_,
  NAND_,
  AND_,
  XOR_,
  NOR_,
  OR_,
  XNOR_,
  DFF_,
  LOGIC_GATE_
};

#endif