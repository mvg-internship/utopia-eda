#include <iostream>
#include <FlexLexer.h>
#include "lexer.c"
#define ASSERT_NEXT_TOKEN(var, tok, err) do { \
    var = get_next_token(); \
    if ((var) != (tok)) { \
        return (err); \
    } \
} while (0)

token_t get_next_token();
error_types parse_bench_file();
error_types parse_id();
error_types parse_parenthesis_io();
error_types parse_parenthesis_id();

static std::size_t place = 0;
static std::size_t line = 0;

token_t get_next_token()
{ 
    token_t val = static_cast<token_t>(scan_token());
    std::cout << " tok: "<< val << " pl: " << place << " ln: " << line << std::endl;
    place += 1;
    return val;
}

error_types parse_bench_file()
{
    token_t token; 
    while(token = get_next_token())
    {
        if (token == INPUT || token == OUTPUT) {
            parse_parenthesis_io();
        } else if (token == ID) {
            parse_id();
        } else {
            return FAILURE_FILE_READING;
        }
    }
    return SUCCESS;
}

error_types parse_id()
{
    token_t token;
    ASSERT_NEXT_TOKEN(token, E, FAILURE_PARSE_ID);
    token = get_next_token();
    if ( token == AND || token == OR || token == NAND || token == NOR ) {
        return parse_parenthesis_id();
    } else if ( token == DFF || token == NOT ) { 
        return parse_parenthesis_io();
    } else 
        return FAILURE_PARSE_ID;    
}

error_types parse_parenthesis_io()
{
    token_t token;
    ASSERT_NEXT_TOKEN(token, LP, FAILURE_IOSTREAM_PARENTHESIS);
    ASSERT_NEXT_TOKEN(token, ID, FAILURE_IOSTREAM_PARENTHESIS);
    ASSERT_NEXT_TOKEN(token, RP, FAILURE_IOSTREAM_PARENTHESIS);
    line += 1;
    return SUCCESS;
}

error_types parse_parenthesis_id()
{
    token_t token;
    ASSERT_NEXT_TOKEN(token, LP, FAILURE_ID_PARENTHESIS);
    ASSERT_NEXT_TOKEN(token, ID, FAILURE_ID_PARENTHESIS);
    while ((token = get_next_token()) == COMMA)
        ASSERT_NEXT_TOKEN(token, ID, FAILURE_ID_PARENTHESIS);
    if (token != RP) 
        return FAILURE_ID_PARENTHESIS;
    line += 1;
    return SUCCESS;
}

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        yyin = fopen( argv[i], "r" );
        std::cout << parse_bench_file() << std::endl;
    }
    
    return 0;
}
