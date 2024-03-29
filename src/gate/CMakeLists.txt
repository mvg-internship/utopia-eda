add_library(Gate OBJECT
  debugger/base_checker.cpp
  debugger/bdd_checker.cpp
  debugger/checker.cpp
  debugger/encoder.cpp
  debugger/miter.cpp
  debugger/rnd_checker.cpp
  debugger/symexec.cpp
  model/gate.cpp
  model/gnet.cpp
  model/gsymbol.cpp
  model/utils.cpp
  optimizer/check_cut.cpp
  optimizer/cone_visitor.cpp
  optimizer/cuts_finder_visitor.cpp
  optimizer/database/abc.cpp
  optimizer/database/abc/rwrUtil.c
  optimizer/links_add_counter.cpp
  optimizer/links_clean.cpp
  optimizer/links_clean_counter.cpp
  optimizer/optimizer.cpp
  optimizer/optimizer_visitor.cpp
  optimizer/rwdatabase.cpp
  optimizer/rwmanager.cpp
  optimizer/strategy/apply_search_optimizer.cpp
  optimizer/strategy/exhaustive_search_optimizer.cpp
  optimizer/strategy/track_strategy.cpp
  optimizer/strategy/zero_optimizer.cpp
  optimizer/substitute_visitor.cpp
  optimizer/tracker_visitor.cpp
  optimizer/ttbuilder.cpp
  optimizer/util.cpp
  optimizer/walker.cpp
  optimizer/tech_map/tech_mapper.cpp
  optimizer/tech_map/tech_map_visitor.cpp
  optimizer/tech_map/strategy/replacement_cut.cpp
  optimizer/tech_map/strategy/simple_techmapper.cpp
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

add_subdirectory(parser/bench)
add_subdirectory(parser/glverilog)

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
