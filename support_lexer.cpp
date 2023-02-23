#include <iostream>
#include <FlexLexer.h>
#include "lexer.c"
#define ASSERT_NEXT_TOKEN(var, tok, err) do { \
    var = get_next_token(); \
    if ((var) != (tok)) { \
        return (err); \
    } \
} while (0)
// /Users/georgryabov/Desktop/ study/lexer/a.out
// /Users/georgryabov/Desktop/\ study/lexer/support_lexer.cpp
//file tokens.h consist of 
/*
enum token_t{
    INPUT = 1,
    OUTPUT,
    DFF,
    NOT,
    AND,
    OR,
    NAND,
    NOR,
    LP, //left parenthesis
    RP, //right paranthesis
    E,  // equal
    ID, //var
    COMMA,
    EOF_TOKEN,
    STRING
};

enum error_types
{
    SUCCESS,
    FAILURE_FILE_READING,
    FAILURE_IOSTREAM_PARENTHESIS,
    FAILURE_ID_PARENTHESIS,
};
*/

token_t get_next_token();
error_types parse_bench_file();
error_types parse_id();
error_types parse_parenthesis_io();
error_types parse_parenthesis_id();

static std::size_t place = 0;
// yytext
token_t get_next_token()
{ 
    // std::cout << "\nplace = " << place << std::endl;
    token_t val = static_cast<token_t>(scan_token());
    std::cout << "This is scan_token: "<< val << std::endl;
    place += 1;
    return val;
}

error_types parse_bench_file()
{
    token_t token; 
    while((token = get_next_token()))
    {
        if(token == INPUT || token == OUTPUT) {
            parse_parenthesis_io();
        } else if(token == ID) {
            parse_id();
        } else {
            return FAILURE_FILE_READING;
        }
    }
    return SUCCESS;
}

error_types parse_id()
{
    token_t token = get_next_token();
    ASSERT_NEXT_TOKEN(token, E, FAILURE_FILE_READING);
    if ((token = get_next_token()) == STRING) {
        return parse_parenthesis_id();
    } else {
        return FAILURE_FILE_READING;
    }
}

error_types parse_parenthesis_io()
{
    token_t token = get_next_token();
    ASSERT_NEXT_TOKEN(token, LP, FAILURE_IOSTREAM_PARENTHESIS);
    ASSERT_NEXT_TOKEN(token, ID, FAILURE_IOSTREAM_PARENTHESIS);
    ASSERT_NEXT_TOKEN(token, RP, FAILURE_IOSTREAM_PARENTHESIS);
    return SUCCESS;
}

error_types parse_parenthesis_id()
{
    token_t token = get_next_token();
    ASSERT_NEXT_TOKEN(token, LP, FAILURE_ID_PARENTHESIS);
    ASSERT_NEXT_TOKEN(token, ID, FAILURE_ID_PARENTHESIS);
    while ((token = get_next_token()) == COMMA)
        ASSERT_NEXT_TOKEN(token, ID, FAILURE_ID_PARENTHESIS);
    ASSERT_NEXT_TOKEN(token, RP, FAILURE_ID_PARENTHESIS);
    return SUCCESS;
}

int main()
{
    yyin = fopen( "s27.txt", "r" );
    std::cout << "error type " << parse_bench_file() << std::endl;
    // while
    // std::cout << "error place " << place << std::endl;
    return 0;
}
