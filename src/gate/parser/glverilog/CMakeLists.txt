find_package(FLEX)
FLEX_TARGET(GateVerilogLexer
  lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/header_file"
)


add_library(GateVerilogParser OBJECT parser_glverilog.cpp
  ${FLEX_GateVerilogLexer_OUTPUTS}
)

target_include_directories(GateVerilogParser
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
  PRIVATE
    ${PROJECT_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)


add_executable(parser.gverilog
  parser_main.cpp
  $<TARGET_OBJECTS:GateVerilogParser>
)

target_include_directories(parser.gverilog
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