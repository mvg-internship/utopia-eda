add_library(Tool OBJECT rtl_context.cpp)

add_library(Utopia::Tool ALIAS Tool)

target_include_directories(Tool PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(Tool
  PUBLIC
    easyloggingpp
    CLI
    Json
    minisat-lib-static
    sqlite3
)