bison_target(RtlRilParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp)
flex_target(RtlRilLexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cpp)
add_flex_bison_dependency(RtlRilLexer RtlRilParser)

add_library(RILParser OBJECT
  builder.cpp
  parser.cpp
  ${FLEX_RtlRilLexer_OUTPUTS}
  ${BISON_RtlRilParser_OUTPUTS}
)
add_library(Utopia::RILParser ALIAS RILParser)

target_include_directories(RILParser
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src

  PRIVATE
    ${PROJECT_BINARY_DIR}/src
)
