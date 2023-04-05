#include <iostream>
#include <FlexLexer.h>
#include <stdlib.h>
#include "lexer.c"
#include <vector>

#define COMPARE(text) do {\
    for (int j = 0; j < strlen(yytext); j++) {\
        if (i.name[j] == yytext[j])\
            k += 1;\
    }\
    if (k == strlen(yytext)) {\
        std::cout << "ERROR IN " << text << " WITH " << yytext << "\tline: " << std::endl;\
        exit(EXIT_FAILURE);\
    }\
} while (false)

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

#define MAPS(definite, type_init) do { \
    char* text = new char[strlen(yytext)]; \
    strcpy(text, yytext); \
    token_map map {text, definite, type_init}; \
    maps.push_back(map); \
} while (false)

struct token_map {
    char* name;
    int definite; // 0 = NOT init, 1 = init
    int type_init; // 0 = input, 1 = output, 2 = function
};

std::vector<token_map> maps;

token_t get_next_token();
void parse_bench_file();
void parse_id();
void parse_parenthesis_io(token_t);
void parse_parenthesis_id(token_t);
void double_defenition(token_t);
void output(token_t);
void input(token_t);

static std::size_t place = 0;
static std::size_t line = 0;

void unknown() {
    std::vector<token_map> maps_upd;
    for(auto i : maps) {
        for(auto j : maps) {
            int k = 0; 
            if(i.definite == 0 && strlen(j.name) == strlen(i.name) && j.definite == 1) {
                    for (int z = 0; z < strlen(j.name); z++) {
                        if (i.name[z] == j.name[z])
                            k += 1;
                    }
                    if (k == strlen(i.name)) {
                        i.definite = 1;
                        maps_upd.push_back(i);
                    }
            }
        }
    }
    for(auto i : maps) {
        if(i.definite == 1)
            maps_upd.push_back(i);
    }
    for(auto i : maps_upd) {
        if(size(maps_upd) != size(maps)) {
            std::cout << "ERROR IN UNKNOWN DEFINITION " << std::endl;
            exit(EXIT_FAILURE);
        }
    }

}

void input() {
    for (auto i : maps) {
        int k = 0;
        if(strlen(yytext) == strlen(i.name) && i.type_init == 0)
            COMPARE("ENTRY INPUT");
    }
} 

void output(token_t type) {
    for (auto i : maps) {
        if (type != OUTPUT) {
            int k = 0;
            if (strlen(yytext) == strlen(i.name) && i.type_init == 1)
                COMPARE("ENTRY INPUT");
        }
    }
}


void double_defenition(token_t type) {
    for (auto i : maps) {
        if(i.definite == 1) {
            int k = 0;
            if (strlen(yytext) == strlen(i.name) && type != OUTPUT && i.type_init != 1)  {
                COMPARE("DOUBLE DEFINITION");
            } else if (strlen(yytext) == strlen(i.name) && type == OUTPUT ) {
                if(i.type_init == 1)
                    COMPARE("DOUBLE DEFINITION");
            }
        }
    }
}

void ASSERT_NEXT_ID(token_t expected_token, char* error_string, token_t type) {
    token_t token = get_next_token(); 
    if (type == INPUT) {
        double_defenition(INPUT);
        MAPS(1, 0);
    } else if (type == OUTPUT) {
        double_defenition(OUTPUT);
        MAPS(1, 1);
    } else {
        output(type);
        MAPS(0, 2);
    } 
    if ( token != expected_token ) { 
        ALERT(error_string); 
    }   
}

token_t get_next_token() { 
    token_t val = static_cast<token_t>(scan_token());
    // std::cout << " token: " << val << "\t" << yytext << "\tline: " << line << " \tplace: " << place << std::endl;
    place += 1;
    return val;
}

void parse_bench_file() {
    while ( token_t token = get_next_token() ) {
        if ( token == INPUT || token == OUTPUT ) {
            parse_parenthesis_io(token);
        } else if (token == ID) {
            double_defenition(ID);
            input();
            MAPS(1, 2);
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
        return parse_parenthesis_id(token);
    } else if ( token == DFF || token == NOT ) { 
        return parse_parenthesis_io(token);
    } else {
        ALERT("function");
    }
}

void parse_parenthesis_io(token_t type) { // type = {INPUT, OUTPUT, ...}
    ASSERT_NEXT_TOKEN( LP, "LP");
    ASSERT_NEXT_ID( ID, "ID", type );
    ASSERT_NEXT_TOKEN( RP, "RP");
    line += 1;
}

void parse_parenthesis_id(token_t type) {
    token_t token;
    ASSERT_NEXT_TOKEN( LP, "LP" );
    ASSERT_NEXT_ID(ID, "ID", type);
    while ( (token = get_next_token()) == COMMA )
        ASSERT_NEXT_ID(ID, "ID", type);
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

    std::vector<char*> txts {"s27.txt", 
                                "dinput.txt",
                                "doutput.txt", 
                                "dddf.txt", 
                                "einput.txt", 
                                "eoutput.txt"
                            };
    yyin = fopen( "s27.txt", "r" );
    parse_bench_file();
    unknown();
    // std::cout << "start\n";
    // for (auto i : maps) { 
    //     std::cout << i.name << " " << i.definite << " " << i.type_init << "\n";
    // }
    std::cout << " end_check\n\n";
    fclose(yyin);
    return 0;
}
