#ifndef BENCH_TOKENS_H
#define BENCH_TOKENS_H
enum Tokens {
    TOK_INPUT = 1,
    TOK_OUTPUT,
    TOK_DFF,
    TOK_NOT,
    TOK_AND,
    TOK_OR,
    TOK_NAND,
    TOK_NOR,
    TOK_LP, //left parenthesis
    TOK_RP, //right paranthesis
    TOK_E,  // equal
    TOK_ID, //var
    TOK_COMMA,
    TOK_N,
    TOK_COMMENT
};
#endif
