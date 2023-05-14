find_package(FLEX)
FLEX_TARGET(BenchLexer
  lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/header_file"
)

add_library(BenchParser OBJECT
  parser.cpp
  ${FLEX_BenchLexer_OUTPUTS}
)

target_include_directories(BenchParser
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
  PRIVATE
    ${PROJECT_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(parser.bench
  parser_main.cpp
  $<TARGET_OBJECTS:BenchParser>
)

target_include_directories(parser.bench
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
  PRIVATE
    ${PROJECT_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(parser.bench
  PRIVATE
    ueda
)