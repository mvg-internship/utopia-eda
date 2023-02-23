enum token_t{
    INPUT = 1,
    OUTPUT,
    // DFF,
    // NOT,
    // AND,
    // OR,
    // NAND,
    // NOR,
    LP, //left parenthesis
    RP, //right paranthesis
    E,  // equal
    ID, //var
    COMMA,
    STRING
};

enum error_types
{
    SUCCESS = 1,
    FAILURE_FILE_READING,
    FAILURE_IOSTREAM_PARENTHESIS,
    FAILURE_ID_PARENTHESIS,
};
