if (NOT TARGET cli11)
  add_library(cli11 INTERFACE)
  target_include_directories(cli11 SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cli11)
endif()

find_package(fmt)

set(fmt_INCLUDE_DIRS ${fmt_INCLUDE_DIR} )

include_directories(${fmt_INCLUDE_DIR})

if (NOT TARGET json)
  add_library(json INTERFACE) # v3.5.0
  target_include_directories(json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json)
endif()

if (NOT TARGET infint)
  add_library(infint INTERFACE)
  target_include_directories(infint SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/infint)
endif()
