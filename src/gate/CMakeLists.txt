add_library(Gate OBJECT
  debugger/encoder.cpp
  debugger/checker.cpp
  debugger/symexec.cpp
  model/gate.cpp
  model/gnet.cpp
  model/gsymbol.cpp
  model/hMetis_formatter.cpp
  premapper/aigmapper.cpp
  premapper/premapper.cpp
  simulator/simulator.cpp
)
add_library(Utopia::Gate ALIAS Gate)

target_include_directories(Gate PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(Gate
  PUBLIC
    minisat-lib-static

  PRIVATE
    Utopia::Util
)
