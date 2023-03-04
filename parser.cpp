#include <iostream>
#include <FlexLexer.h>
#include <stdlib.h>
#include "lexer.c"

#define ALERT(comp) do { \
    std::cout << " expected the " << comp << " in the expr in the line " << line << ". Сaught " << yytext << std::endl; \
    exit(EXIT_FAILURE); \
} while (false) 

#define ASSERT_NEXT_TOKEN(token, comp, err) do { \
    if ( assert_next_token(token, comp, err) == err ) { \
        if ( comp == LP )   \
            ALERT("'LP'");  \
        else if ( comp == ID )   \
            ALERT("'ID'");  \
        else if ( comp == RP )   \
            ALERT("'RP'");  \
        else if ( comp == E)    \
            ALERT("'E'");    \
    }   \
} while (false)

// cd build/
// cmake ..
// make

token_t get_next_token();
error_types parse_bench_file();
error_types parse_id();
error_types parse_parenthesis_io();
error_types parse_parenthesis_id();

static std::size_t place = 0;
static std::size_t line = 0;

error_types assert_next_token(token_t var, token_t tok, error_types err) {
    var = get_next_token(); 
    if ( var != tok )
        return ( err ); 
    return SUCCESS; 
}

token_t get_next_token()
{ 
    token_t val = static_cast<token_t>(scan_token());
    std::cout << " tok: " << val << " " << yytext << " pl: " << place << " ln: " << line << std::endl;
    place += 1;
    return val;
}

error_types parse_bench_file()
{
    token_t token; 
    while( token = get_next_token() )
    {
        if ( token == INPUT || token == OUTPUT ) {
            parse_parenthesis_io();
        } else if ( token == ID ) {
            parse_id();
        } else {
            ALERT("'INPUT', 'OUTPUT' or 'ID'");
        }
    }
}

error_types parse_id()
{
    token_t token;
    ASSERT_NEXT_TOKEN( token, E, FAILURE_PARSE_ID );
    token = get_next_token();
    if ( token == AND || token == OR || token == NAND || token == NOR ) {
        return parse_parenthesis_id();
    } else if ( token == DFF || token == NOT ) { 
        return parse_parenthesis_io();
    } else {
        ALERT("function");
    }
}

error_types parse_parenthesis_io()
{
    token_t token;
    ASSERT_NEXT_TOKEN( token, LP, FAILURE_IOSTREAM_PARENTHESIS );
    ASSERT_NEXT_TOKEN( token, ID, FAILURE_IOSTREAM_PARENTHESIS );
    ASSERT_NEXT_TOKEN( token, RP, FAILURE_IOSTREAM_PARENTHESIS );
    line += 1;
    return SUCCESS;
}

error_types parse_parenthesis_id()
{
    token_t token;
    ASSERT_NEXT_TOKEN( token, LP, FAILURE_ID_PARENTHESIS );
    ASSERT_NEXT_TOKEN( token, ID, FAILURE_ID_PARENTHESIS );
    while ( (token = get_next_token()) == COMMA )
        ASSERT_NEXT_TOKEN( token, ID, FAILURE_ID_PARENTHESIS );
    if ( token != RP ) {
        ALERT("'RP'");
    }
    line += 1;
    return SUCCESS;
}

int main(int argc, char* argv[])
{
    // for (int i = 1; i < argc; i++) {
    //     yyin = fopen( argv[i], "r" );
    //     std::cout << parse_bench_file() << std::endl;
    // }

    yyin = fopen( "s27.txt", "r" );
    std::cout << parse_bench_file() << std::endl;
    return 0;
}
