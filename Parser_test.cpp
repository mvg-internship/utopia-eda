#include <iostream>
#include <vector>
#include <FlexLexer.h>
#include "lex.yy.c"

/*
some defenicion in Verilog

T = {module, endmodule, input, output, wire, not,and, nand, end, or, xor, /n}
N = { =, (, )}

stmt ->  module* EOF
module -> MODULE name "(" name(,name)*")"; decl* expr* | ENDMODULE
decl -> (input|output|wire)?[NUM:NUM]? name(,name)* ;// тут мы будем проверять, есть ли далее скобка, так как она опциональна
expr -> name name (arg*,);
arg-> name ?[NUM]?; // тут мы будем проверять, есть ли далее скобка, так как она опциональна
*/

static std::vector<token_t> store;

token_t get_next_token();
kind_of_error parse_gatelevel_verilog();
kind_of_error parse_module();
kind_of_error parse_decl(token_t);
kind_of_error parse_expr(token_t);
kind_of_error parse_arg(token_t);
kind_of_error parse_name_list(token_t&, token_t);

#define ASSERT_NEXT_TOKEN(var, tok, err) \
  do                                     \
  {                                      \
    var = get_next_token();              \
    if ((var) != (tok))                  \
    {                                    \
      std::cout<<"Error = " << (err) << std::endl;   \
      return (err);                      \
    }                                    \
  } while (0)

static std::size_t place = 0;
static std::size_t line = 0;

token_t get_next_token()
{ 
  token_t val = static_cast<token_t>( scan_token());
  std::cout << " type: "<< val << " tok: "<< yytext << " pl: " << place << " ln: " <<  line << std::endl;
  place += 1;
  return val;
}

kind_of_error parse_gatelevel_verilog()
{
  token_t tok;
  kind_of_error rc = SUCCESS;
  ASSERT_NEXT_TOKEN(tok, MODULE, FAILURE_IN_GATE_LEVEL_VERILOG);
  rc = parse_module();
}

kind_of_error parse_module()
{
  token_t tok;
  kind_of_error rc = SUCCESS;

  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_MODULE_NAME);
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
  rc = parse_name_list(tok, RBRACE);

  ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_NAME);
  line += 1;

  while (rc == SUCCESS && tok != ENDMODULE)
  {
    switch (tok)
    {
    case INPUT:
    case OUTPUT:
    case WIRE:
      rc = parse_decl(tok);
      break;
    case STRING:
      rc = parse_expr(tok);
      break;
    default:
      rc = FAILURE_IN_MODULE_INCAPTULATION;
      break;
    }
  }
  
}

kind_of_error parse_decl(token_t tok)
{
  tok = get_next_token();
  kind_of_error rc = SUCCESS;
  switch (tok)
  {
  case LBRACKET:
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_DECL);
    rc = parse_name_list(tok, SEMICOLON);
    line += 1;
    break;
  case STRING:
    rc = parse_name_list(tok, SEMICOLON);
    line += 1;
    break;
  default:
    rc = FAILURE_IN_DECL;
    break;
  }
 
  return rc;
}

kind_of_error parse_expr(token_t tok)
{
  kind_of_error rc = SUCCESS;

  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_EXPR);
  line += 1;

  rc = parse_arg(tok);

  return rc;
}

kind_of_error parse_arg(token_t tok)
{
  kind_of_error rc = SUCCESS;

  while (tok != RBRACE && rc == SUCCESS)
  {
    if (tok != STRING)
    {
      rc = FAILURE_IN_ARG;
      break;
    }
    else
    {
      store.push_back(tok);
    }

    tok = get_next_token();

    switch (tok)
    {
    case LBRACKET:
      if ((tok = get_next_token()) != NUM)
      {
        rc = FAILURE_IN_ARG;
        break;
      }
      else
      {
        store.push_back(tok);
      }

      if ((tok = get_next_token()) != RBRACKET)
        rc = FAILURE_IN_ARG;
      break;
    case COMMA:
      tok = get_next_token();
      break;
    default:
      rc = FAILURE_IN_ARG;
      break;
    }
  }
  if ((tok = get_next_token()) != SEMICOLON)
    rc = FAILURE_IN_ARG;
  return rc;
}

kind_of_error parse_name_list(token_t &tok, token_t separate_tok)
{
  kind_of_error rc = SUCCESS;
  while (tok != separate_tok && rc == SUCCESS)
  {
    ASSERT_NEXT_TOKEN(tok, COMMA, FAILURE_IN_PARSE_NAME_LIST);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_PARSE_NAME_LIST);
  }
  return rc;
}

int main(int argc, char *argv[]) {
    yyin = fopen(argv[1], "r");
    return parse_gatelevel_verilog();
}