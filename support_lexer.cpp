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
// yytext
token_t get_next_token()
{ 
    // std::cout << "\nplace = " << place << std::endl;
    std::cout << " black box 1 " << scan_token() << "  " << std::endl;
    std::cout << " black box 2 " << scan_token() << "  " << std::endl;
    std::cout << " black box 3 " << scan_token() << "  " << std::endl;
    std::cout << " black box 4 " << scan_token() << "  " << std::endl;
    std::cout << " black box 5 " << scan_token() << "  " << std::endl;
    token_t val = static_cast<token_t>(scan_token());
    std::cout << " token: "<< val << " " << place << std::endl;
    place += 1;
    return val;
}

error_types parse_bench_file()
{
    token_t token; 
    while((token = get_next_token()))
    {
         std::cout << " rkalfd " << token << " " << place << std::endl;
        if(token == INPUT || token == OUTPUT) {
            parse_parenthesis_io();
        } else if(token == ID) {
            std::cout << " askf " << token;
            parse_id();
        } else {
            return FAILURE_FILE_READING;
        }
    }
    return SUCCESS;
}

error_types parse_id()
{
     std::cout << " sdjnfjnfdasnpfasndf; ";
    token_t token;
    // ASSERT_NEXT_TOKEN(token, E, FAILURE_PARSE_ID);
    token = get_next_token();
    if ( token == AND 
            || token == OR 
            || token == NAND 
            || token== NOR
       ) 
        return parse_parenthesis_id();
    else if ( token == DFF 
                || token == NOT )
        return parse_parenthesis_io();
     else 
        return FAILURE_PARSE_ID;    
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
    std::cout << "END OF P_ID " << token << std::endl;
    return SUCCESS;
}

int main()
{
    yyin = fopen( "s27.txt", "r" );
    std::cout << parse_bench_file() << std::endl;
    // while
    // std::cout << "error place " << place << std::endl;
    return 0;
}
