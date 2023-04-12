#include <iostream>
#include <unordered_map>
#include <vector>
#include "lex.yy.c"
#include <FlexLexer.h>

/*
some defenicion in Verilog

T = {module, endmodule, input, output, wire, not,and, nand, end, or, xor, /n}
N = { =, (, )}

stmt ->  module* EOF
module -> MODULE name "(" name(,name)*")"; decl* expr* | ENDMODULE
decl -> (input|output|wire)?[NUM:NUM]? name(,name)* ;// тут мы будем проверять,
есть ли далее скобка, так как она опциональна expr -> name name (arg*,); arg->
name ?[NUM]?; // тут мы будем проверять, есть ли далее скобка, так как она
опциональна
*/

class SymbolTable {
private:
  struct Symbol {
    familyInfo parent;
    familyInfo child;
  };
  std::unordered_map<std::string, Symbol> table;

public:
  void addSymbol(const std::string &name, familyInfo parent = VOID_,
                 familyInfo child = VOID_) {
    Symbol symbol;
    symbol.parent = parent;
    symbol.child  = child;
    table[name]   = symbol;
  }

  void removeSymbol(const std::string &name) {
    table.erase(name);
  }

  void changeParent(const std::string &name, familyInfo parent) {
    table[name].parent = parent;
  }

  void changeChild(const std::string &name, familyInfo child) {
    table[name].child = child;
  }

  familyInfo findParent(const std::string &name) {
    return table[name].parent;
  }

  familyInfo findChild(const std::string &name) {
    return table[name].child;
  }
};

void AsserrtVariable(const std::string &name, familyInfo familyType,
                     SymbolTable table) {
  switch (familyType) {
  case WIRE_:
  case MODULE_:
    if (table.findParent(name) == familyInfo::VOID_ &&
        table.findChild(name) == familyInfo::VOID_) {
      table.addSymbol(name, familyType, familyInfo::VOID_);
    } else {
      std::cout << "This variable declorated twice: " << name << std::endl
                << "line: " << yylineno << std::endl;
    }

    break;
  case INPUT_:
  case OUTPUT_:
    if (table.findParent(name) == MODULE_ &&
        table.findChild(name) == familyInfo::VOID_) {
      table.changeChild(name, familyType);
    } else {
      std::cout << "Invalid decloration " << name << std::endl
                << "line: " << yylineno << std::endl;
    }

    break;
  case ASSIGN_:
     if(table.findParent(name) == MODULE_ && table.findChild(name) != ASSIGN_) {
      table.changeChild(name, ASSIGN_);
     }else{
      std::cout << "Invalid decloration " << name << std::endl
                << "line: " << yylineno << std::endl;
     }

    break;
  default:

    break;
  }
};

token_t       get_next_token();
kind_of_error parse_gatelevel_verilog();
kind_of_error parse_module(token_t &);
kind_of_error parse_decl(token_t &, SymbolTable &);
kind_of_error parse_expr(token_t &, SymbolTable &);
kind_of_error parse_assign_parts(token_t &, SymbolTable &);
kind_of_error parse_assign(token_t &, SymbolTable &);
kind_of_error parse_arg(token_t &, SymbolTable &);
kind_of_error parse_name_list(token_t &, token_t, familyInfo, SymbolTable &);

#define DEBUGTOKEN(tok, msg)                                                   \
  printf("%s:%d: %s: token '%s' (%d)\n", __FILE__, __LINE__, (msg), yytext,    \
         (tok))

#define ASSERT_NEXT_TOKEN(var, tok, err)                                       \
  do {                                                                         \
    var = get_next_token();                                                    \
    if ((var) != (tok)) {                                                      \
      std::cout << __FILE__ << __LINE__ << yytext << " tok = " << tok          \
                << std::endl;                                                  \
      std::cout << "Error (in Assert) = " << (err) << std::endl;               \
      return (err);                                                            \
    }                                                                          \
  } while (0)

static std::size_t place = 0;

token_t get_next_token() {
  token_t val = static_cast<token_t>(scan_token());
  std::cout << " type: " << val << " tok: " << yytext << " pl: " << place
            << " ln: " << yylineno << std::endl;
  place += 1;
  return val;
}

kind_of_error parse_gatelevel_verilog() {

  token_t       tok = START;
  kind_of_error rc  = SUCCESS;
  while (tok != EOF_TOKEN) {
    DEBUGTOKEN(tok, "Verilog loop");
    ASSERT_NEXT_TOKEN(tok, MODULE, FAILURE_IN_GATE_LEVEL_VERILOG);
    rc = parse_module(tok);
  }
  std::cout << "Error! "
            << "type = " << rc << std::endl;
  return rc;
}

kind_of_error parse_module(token_t &tok) {
  kind_of_error rc = SUCCESS;
  SymbolTable   table;

  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_MODULE_NAME);
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_MODULE_NAME);
  table.addSymbol(yytext, MODULE_);
  rc = parse_name_list(tok, RBRACE, MODULE_, table);
  ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_NAME);
  tok = get_next_token();

  while (rc == SUCCESS && tok != ENDMODULE) {
    DEBUGTOKEN(tok, "Module loop");

    switch (tok) {
    case INPUT:
    case OUTPUT:
    case WIRE:
      rc = parse_decl(tok, table);
      break;
    case ASSIGN:
      rc = parse_assign(tok, table);
      break;
    case STRING:
      rc = parse_expr(tok, table);
      break;
    default:
      rc = FAILURE_IN_MODULE_INCAPTULATION;
      break;
    }
  }

  delete &table;
  return rc;
}

kind_of_error parse_decl(token_t &tok, SymbolTable &table) {
  //Сохраняем тип деклорации. Ничего умнее я не придумал
  familyInfo familyType = familyInfo::VOID_;
  switch (tok)
  {
  case INPUT:
    familyType = INPUT_;
    break;
  case OUTPUT:
    familyType = OUTPUT_;
    break;
  case WIRE:
    familyType = WIRE_;
    break;
  default:
    break;
  }
  tok              = get_next_token();
  kind_of_error rc = SUCCESS;
  switch (tok) {
  case LBRACKET:
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_DECL);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_DECL);
    rc  = parse_name_list(tok, SEMICOLON, familyType, table);
    tok = get_next_token();

    break;
  case STRING:
    rc  = parse_name_list(tok, SEMICOLON, familyType, table);

    tok = get_next_token();
    break;
  default:
    rc = FAILURE_IN_DECL;

    break;
  }

  return rc;
}

kind_of_error parse_expr(token_t &tok, SymbolTable &table) {
  kind_of_error rc = SUCCESS;

  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_EXPR);
  ASSERT_NEXT_TOKEN(tok, LBRACE, FAILURE_IN_EXPR);

  DEBUGTOKEN(tok, "Parse expr begin");
  rc = parse_arg(tok, table);
  DEBUGTOKEN(tok, "Parse expr end");
  return rc;
}

kind_of_error parse_assign_parts(token_t &tok, SymbolTable &table) {
  kind_of_error rc = SUCCESS;
  ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
  if(table.findParent(yytext) == WIRE_ && table.findChild(yytext) == familyInfo::VOID_){
    table.changeChild(yytext, ASSIGN_);
  }
  tok = get_next_token();
  switch (tok) {
  case LBRACKET:
    std::cout << "aa" << std::endl;
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, COLON, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, EQUALS, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, LFIGURNAYA, FAILURE_IN_ASSIGN);
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
    rc = parse_name_list(tok, RFIGURNAYA, ASSIGN_, table);
    std::cout << __FILE__ << __LINE__ << yytext << " end of lbracket case "
              << "tok =" << tok << std::endl;
    break;
  case EQUALS:
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ASSIGN);
    std::cout << __FILE__ << __LINE__ << yytext << "end of equals case"
              << "tok =" << tok << std::endl;
    break;
  default:
    break;
  }

  return rc;
}

kind_of_error parse_assign(token_t &tok, SymbolTable &table) {
  kind_of_error rc = SUCCESS;
  while (tok != SEMICOLON) {
    rc  = parse_assign_parts(tok, table);
    tok = get_next_token();
    if (tok != COMMA && tok != SEMICOLON) {
      rc = FAILURE_IN_ASSIGN;
    }
  }
  tok = get_next_token();
  return rc;
}

kind_of_error parse_arg(token_t &tok, SymbolTable &table) {
  kind_of_error rc = SUCCESS;

  while (tok != RBRACE && rc == SUCCESS) {
    ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_ARG);

    tok = get_next_token();
    DEBUGTOKEN(tok, "ARG  loop");

    if (tok == LBRACKET) {
      ASSERT_NEXT_TOKEN(tok, NUM, FAILURE_IN_ARG); 
      ASSERT_NEXT_TOKEN(tok, RBRACKET, FAILURE_IN_ARG);
      tok = get_next_token();
    }
    if (tok != COMMA && tok != RBRACE) {
      rc = FAILURE_IN_ARG;
    }
  }
  ASSERT_NEXT_TOKEN(tok, SEMICOLON, FAILURE_IN_MODULE_INCAPTULATION);
  tok = get_next_token();
  return rc;
}

kind_of_error parse_name_list(token_t &tok, token_t separate_tok,
                              familyInfo familyType, SymbolTable &table) {
  kind_of_error rc = SUCCESS;

  while (tok != separate_tok && rc == SUCCESS) {
    tok = get_next_token();
    switch (tok) {
    case COMMA:
      ASSERT_NEXT_TOKEN(tok, STRING, FAILURE_IN_PARSE_NAME_LIST);
      AsserrtVariable(yytext, familyType, table);
      break;

    default:
      if (tok != separate_tok) {

        rc = FAILURE_IN_PARSE_NAME_LIST;
      } else {
        break;
      }
      break;
    }
  }
  return rc;
}

int main(int argc, char *argv[]) {
  yyin = fopen(argv[1], "r");
  return parse_gatelevel_verilog();
}