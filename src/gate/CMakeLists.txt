add_library(Gate OBJECT
  debugger/encoder.cpp
  debugger/checker.cpp
  debugger/rnd_checker.cpp
  debugger/symexec.cpp
  debugger/miter.cpp
  debugger/RND_checker.cpp
  debugger/bdd_checker.cpp
  debugger/BDD_checker.cpp
  debugger/base_checker.cpp
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

target_include_directories(Gate PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(Gate
  PUBLIC
    minisat-lib-static
    Cudd::Cudd
    sqlite3

  PRIVATE
    Utopia::Util
)
