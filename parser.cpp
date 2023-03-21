#include <iostream>
#include <FlexLexer.h>
#include <stdlib.h>
#include "lexer.c"

#define ALERT(error_string) do { \
    std::cout << " expected '" << error_string << "'\tline: " << line << "\tСaught " << yytext << std::endl; \
    exit(EXIT_FAILURE); \
} while (false) 

#define ASSERT_NEXT_TOKEN(expected_token, error_string) do { \
    token_t token = get_next_token(); \
    if ( token != expected_token ) { \
        ALERT(error_string); \
    }   \
} while (false)

struct symbol_struct {
    char* name;
    bool type_usage;

};

token_t get_next_token();
void parse_bench_file();
void parse_id();
void parse_parenthesis_io();
void parse_parenthesis_id();

static std::size_t place = 0;
static std::size_t line = 0;

token_t get_next_token()
{ 
    token_t val = static_cast<token_t>(scan_token());
    std::cout << " token: " << val << "\t" << yytext << "\tline: " << line << " \tplace: " << place << std::endl;
    place += 1;
    return val;
}

void parse_bench_file()
{
    while( token_t token = get_next_token() )
    {
        if ( token == INPUT || token == OUTPUT ) {
            parse_parenthesis_io();
        } else if ( token == ID ) {
            parse_id();
        } else {
            ALERT("INPUT', 'OUTPUT' or 'ID");
        }
    }
    std::cout << "END OF FILE.\n";
}

void parse_id()
{
    ASSERT_NEXT_TOKEN( E, "E" );
    token_t token = get_next_token();
    if ( token == AND || token == OR || token == NAND || token == NOR ) {
        return parse_parenthesis_id();
    } else if ( token == DFF || token == NOT ) { 
        return parse_parenthesis_io();
    } else {
        ALERT("function");
    }
}

void parse_parenthesis_io()
{
    ASSERT_NEXT_TOKEN( LP, "LP" );
    ASSERT_NEXT_TOKEN( ID, "ID" );
    ASSERT_NEXT_TOKEN( RP, "RP" );
    line += 1;
}

void parse_parenthesis_id()
{
    token_t token;
    ASSERT_NEXT_TOKEN( LP, "LP" );
    ASSERT_NEXT_TOKEN( ID, "ID" );
    while ( (token = get_next_token()) == COMMA )
        ASSERT_NEXT_TOKEN( ID, "ID" );
    if ( token != RP ) {
        ALERT("'RP'");
    }
    line += 1;
}

int main(int argc, char* argv[])
{
    // for (int i = 1; i < argc; i++) {
    //     yyin = fopen( argv[i], "r" );
    //     std::cout << parse_bench_file() << std::endl;
    // }

    yyin = fopen( "s27.txt", "r" );
    parse_bench_file();
    return 0;
}
