add_subdirectory(parser/ril)

add_library(RTL OBJECT
  compiler/compiler.cpp
  library/arithmetic.cpp
  library/flibrary.cpp
  model/fsymbol.cpp
  model/net.cpp
  model/pnode.cpp
  model/vnode.cpp
)
add_library(Utopia::RTL ALIAS RTL)

target_include_directories(RTL PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(RTL
  PUBLIC
    Utopia::Util
    Utopia::Gate
)
