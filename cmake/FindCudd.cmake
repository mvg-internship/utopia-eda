set(Cudd_DEFAULT_SOURCE_DIR "../cudd")

find_path(Cudd_INCLUDE_DIR "cudd.h" PATHS ${Cudd_DEFAULT_SOURCE_DIR} PATH_SUFFIXES cudd)
find_path(CuddObj_INCLUDE_DIR "cuddObj.hh" PATHS ${Cudd_DEFAULT_SOURCE_DIR} PATH_SUFFIXES cplusplus)

if (NOT Cudd_LIBRARY)
    find_library(Cudd_LIBRARY cudd PATHS ${Cudd_DEFAULT_SOURCE_DIR} PATH_SUFFIXES cudd/.libs)
endif()
if (NOT CuddObj_LIBRARY)
    find_library(CuddObj_LIBRARY obj PATHS ${Cudd_DEFAULT_SOURCE_DIR} PATH_SUFFIXES cplusplus/.libs)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Cudd
    REQUIRED_VARS
        Cudd_LIBRARY
        Cudd_INCLUDE_DIR
)
find_package_handle_standard_args(CuddObj
    REQUIRED_VARS
        CuddObj_LIBRARY
        CuddObj_INCLUDE_DIR
)

if(Cudd_FOUND)
    set(Cudd_INCLUDE_DIRS ${Cudd_INCLUDE_DIR})

    if(NOT Cudd_LIBRARIES)
        set(Cudd_LIBRARIES ${Cudd_LIBRARY})
    endif()

    if (NOT TARGET Cudd::Cudd)
        add_library(Cudd::Cudd UNKNOWN IMPORTED)
        set_target_properties(Cudd::Cudd PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${Cudd_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${Cudd_LIBRARY}")
    endif()
endif()
if(CuddObj_FOUND)
    set(CuddObj_INCLUDE_DIRS ${CuddObj_INCLUDE_DIR})

    if(NOT CuddObj_LIBRARIES)
        set(CuddObj_LIBRARIES ${CuddObj_LIBRARY})
    endif()

    if (NOT TARGET Cudd::CuddObj)
        add_library(Cudd::CuddObj UNKNOWN IMPORTED)
        set_target_properties(Cudd::CuddObj PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CuddObj_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${CuddObj_LIBRARY}")
    endif()
endif()

target_link_libraries(Cudd::CuddObj INTERFACE Cudd::Cudd)
