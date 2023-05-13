find_package(FLEX)
FLEX_TARGET(GateVerilogLexer
  Lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/header_file"
)

add_executable(parser.gverilog
  Parser_test.cpp
  ${FLEX_GateVerilogLexer_OUTPUTS}
)

add_library(GateVerilogParser OBJECT
  Parser_test.cpp
  ${FLEX_GateVerilogLexer_OUTPUTS}
)

target_include_directories(GateVerilogParser
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src

  PRIVATE
    ${PROJECT_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
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
    ${CMAKE_CURRENT_BINARY_DIR}
)