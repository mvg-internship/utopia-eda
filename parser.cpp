#include <iostream>
#include <FlexLexer.h>
#include <stdlib.h>
#include "lexer.c"
#include <vector>

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

struct token_map {
    char* name;
    int definite; // 0 = was NOT init, 1 = was init
    int type_init; // 0 = input, 1 = output, 2 = function
};

std::vector<token_map> maps;

token_t get_next_token();
void parse_bench_file();
void parse_id();
void parse_parenthesis_io(token_t);
void parse_parenthesis_id();

static std::size_t place = 0;
static std::size_t line = 0;

void ASSERT_NEXT_ID(token_t expected_token, char* error_string, token_t type) {
    token_t token = get_next_token(); 
    std::cout << "YYTEXTT" << yytext << std::endl;
    char* text = yytext;
    std::cout << "TEXT" << text << std::endl;
    if (type == INPUT) {
        token_map map {text, 1, 0}; 
        maps.push_back(map); 
    } else if (type == OUTPUT) {
        token_map map {text, 1, 1}; 
        maps.push_back(map); 
    } else if (type == DFF) { 
        token_map map {text, 0, 2};
        maps.push_back(map); 
    } else if (type == NOT) { 
        token_map map {text, 0, 2}; 
        maps.push_back(map); 
    } 
    if ( token != expected_token ) { 
        ALERT(error_string); 
    }   
}

token_t get_next_token() { 
    token_t val = static_cast<token_t>(scan_token());
    std::cout << " token: " << val << "\t" << yytext << "\tline: " << line << " \tplace: " << place << std::endl;
    place += 1;
    return val;
}

void parse_bench_file() {
    while ( token_t token = get_next_token() ) {
        if ( token == INPUT || token == OUTPUT ) {
            parse_parenthesis_io(token);
        } else if ( token == ID ) {
            parse_id();
        } else {
            ALERT("INPUT', 'OUTPUT' or 'ID");
        }
    }
    std::cout << "END OF FILE.\n";
}

void parse_id() {
    ASSERT_NEXT_TOKEN( E, "E" );
    token_t token = get_next_token();
    if ( token == AND || token == OR || token == NAND || token == NOR ) {
        return parse_parenthesis_id();
    } else if ( token == DFF || token == NOT ) { 
        return parse_parenthesis_io(token);
    } else {
        ALERT("function");
    }
}

void parse_parenthesis_io(token_t type) { // type = {INPUT, OUTPUT}
    ASSERT_NEXT_TOKEN( LP, "LP");
    ASSERT_NEXT_ID( ID, "ID", type );
    ASSERT_NEXT_TOKEN( RP, "RP");
    line += 1;
}

void parse_parenthesis_id() {
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

int main(int argc, char* argv[]) {
    // for (int i = 1; i < argc; i++) {
    //     yyin = fopen( argv[i], "r" );
    //     std::cout << parse_bench_file() << std::endl;
    // }
    yyin = fopen( "s27.txt", "r" );
    parse_bench_file();
    for (auto i : maps) {
        std::cout << i.name << std::endl;
    } 
    return 0;
}













// << " " << i.definite << " " << i.type_init 
