add_library(Gate OBJECT
  debugger/encoder.cpp
  debugger/checker.cpp
  debugger/symexec.cpp
  model/gate.cpp
  model/gnet.cpp
  model/gsymbol.cpp
  model/utils.cpp
  optimizer/rwdatabase.cpp
  premapper/aigmapper.cpp
  premapper/migmapper.cpp
  premapper/premapper.cpp
  premapper/xagmapper.cpp
  premapper/xmgmapper.cpp
  printer/graphml.cpp
  printer/dot.cpp
  parser/gate_verilog_parser.cpp
  simulator/simulator.cpp
  transformer/bdd.cpp
  transformer/hmetis.cpp
  library/liberty/translate.cpp
  library/liberty/net_data.cpp
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
    Yosys::Yosys
)
