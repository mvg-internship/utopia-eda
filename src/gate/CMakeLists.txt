add_library(Gate OBJECT
  debugger/encoder.cpp
  debugger/checker.cpp
  debugger/symexec.cpp
  model/gate.cpp
  model/gnet.cpp
  model/gsymbol.cpp
  optimizer/rwdatabase.cpp
  premapper/aigmapper.cpp
  premapper/premapper.cpp
  simulator/simulator.cpp
  transformer/bdd.cpp
  transformer/hmetis.cpp
)
add_library(Utopia::Gate ALIAS Gate)

add_library(sqlite3 optimizer/sqlite/sqlite3.c)

set_target_properties(sqlite3 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES
  ${PROJECT_SOURCE_DIR}/src/gate/optimizer/sqlite)

target_include_directories(Gate PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(Gate
  PUBLIC
    minisat-lib-static
    Cudd::Cudd
    sqlite3

  PRIVATE
    Utopia::Util
)
