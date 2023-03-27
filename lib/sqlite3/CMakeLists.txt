add_library(sqlite3 sqlite3.c sqlite3-bind.c)

set_target_properties(sqlite3 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES
  ${PROJECT_SOURCE_DIR}/lib/sqlite3)
target_link_libraries(sqlite3 ${CMAKE_DL_LIBS})
