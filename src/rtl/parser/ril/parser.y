//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

%{
  #include <string>
  #include <iostream>

  #include "rtl/parser/ril/builder.h"

  using namespace eda::rtl;
  using namespace eda::rtl::parser::ril;

  extern int rrlineno;
  extern char* rrtext;

  extern int rrlex(void);

  void rrerror(const char *error) {
    std::cerr << "Error(" << rrlineno << "): " << error << std::endl;
    std::cerr << rrtext << std::endl;
  }
%}

%define api.prefix {rr}
%define api.value.type {std::string*}
%define api.token.prefix {TOK_}
%token
  INPUT
  OUTPUT
  WIRE
  REG
  IF
  POSEDGE
  NEGEDGE
  LEVEL0
  LEVEL1
  AT
  STAR
  ASSIGN
  NOT
  AND
  OR
  XOR
  ADD
  SUB
  SEMI
  LBRACK
  RBRACK
  LCURLY
  RCURLY
  NUM
  VAL
  VAR
  TYPE
  OTHER
;

%%

model:
  { Builder::get().start_model(); }
  items
;

items:
  %empty
| items item
;

item:
    decl
  | proc
;

decl:
  INPUT TYPE[t] VAR[n] SEMI {
    Builder::get().add_decl(Variable::WIRE, Variable::INPUT, *$n, *$t);
    delete $t; delete $n;
  }
| OUTPUT TYPE[t] VAR[n] SEMI {
    Builder::get().add_decl(Variable::WIRE, Variable::OUTPUT, *$n, *$t);
    delete $t; delete $n;
  }
| WIRE TYPE[t] VAR[n] SEMI {
    Builder::get().add_decl(Variable::WIRE, Variable::INNER, *$n, *$t);
    delete $t; delete $n;
  }
| REG TYPE[t] VAR[n] SEMI {
    Builder::get().add_decl(Variable::REG, Variable::INNER, *$n, *$t);
    delete $t; delete $n;
  }
;

proc:
  { Builder::get().start_proc(); } 
  AT LBRACK event RBRACK guard LCURLY action RCURLY
  { Builder::get().end_proc(); }
;

event:
  POSEDGE LBRACK VAR[s] RBRACK {
    Builder::get().set_event(Event::POSEDGE, *$s);
    delete $s;
  }
| NEGEDGE LBRACK VAR[s] RBRACK {
    Builder::get().set_event(Event::NEGEDGE, *$s);
    delete $s;
  }
| LEVEL0 LBRACK VAR[s] RBRACK {
    Builder::get().set_event(Event::LEVEL0, *$s);
    delete $s;
  }
| LEVEL1 LBRACK VAR[s] RBRACK {
    Builder::get().set_event(Event::LEVEL1, *$s);
    delete $s;
  }
| STAR {
    Builder::get().set_event(Event::ALWAYS, "");
  }
;

guard:
  %empty {
    Builder::get().set_guard("");
  }
| IF LBRACK VAR[g] RBRACK {
    Builder::get().set_guard(*$g);
    delete $g;
  }
;

action:
  %empty
| action assign
;

assign:
  VAR[f] ASSIGN VAL[c] SEMI {
    Builder::get().add_assign(NOP, *$f, {*$c});
    delete $f; delete $c;
  }
| VAR[f] ASSIGN VAR[x] SEMI {
    Builder::get().add_assign(NOP, *$f, {*$x});
    delete $f; delete $x;
  }
| VAR[f] ASSIGN NOT VAR[x] SEMI {
    Builder::get().add_assign(NOT, *$f, {*$x});
    delete $f; delete $x;
  }
| VAR[f] ASSIGN VAR[x] AND VAR[y] SEMI {
    Builder::get().add_assign(AND, *$f, {*$x, *$y});
    delete $f; delete $x; delete $y;
  }
| VAR[f] ASSIGN VAR[x] OR  VAR[y] SEMI {
    Builder::get().add_assign(OR,  *$f, {*$x, *$y});
    delete $f; delete $x; delete $y;
  }
| VAR[f] ASSIGN VAR[x] XOR VAR[y] SEMI {
    Builder::get().add_assign(XOR, *$f, {*$x, *$y});
    delete $f; delete $x; delete $y;
  }
| VAR[f] ASSIGN VAR[x] ADD VAR[y] SEMI {
    Builder::get().add_assign(ADD, *$f, {*$x, *$y});
    delete $f; delete $x; delete $y;
  }
| VAR[f] ASSIGN VAR[x] SUB VAR[y] SEMI {
    Builder::get().add_assign(SUB, *$f, {*$x, *$y});
    delete $f; delete $x; delete $y;
  }
| VAR[f] ASSIGN VAR[x] STAR VAR[y] SEMI {
    Builder::get().add_assign(MUL, *$f, {*$x, *$y});
    delete $f; delete $x; delete $y;
  }
;

%%
