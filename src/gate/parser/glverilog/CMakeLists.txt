find_package(FLEX)
FLEX_TARGET(GateVerilogLexer Lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

add_executable(parser.gverilog
  Parser_test.cpp
  ${FLEX_GateVerilogLexer_OUTPUTS}
)
target_link_libraries(parser.gverilog
  PRIVATE
    ueda
)

target_include_directories(parser.gverilog
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src

  PRIVATE
    ${PROJECT_BINARY_DIR}/src
)