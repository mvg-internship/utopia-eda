cmake_minimum_required(VERSION 3.24.2)
find_package(FLEX)
FLEX_TARGET(BenchLexer 
  lexer.l 
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.c 
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/headerFile"
)

add_executable(parser.bench
  parser.cpp
  ${FLEX_BenchLexer_OUTPUTS}
)
target_link_libraries(parser.bench
  PRIVATE
    ueda
)

target_include_directories(parser.bench
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src

  PRIVATE
    ${PROJECT_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)
