cmake_minimum_required(VERSION 3.1)
project(lorina LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(LORINA_EXAMPLES "Build examples" ON)
option(LORINA_TEST "Build tests" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)

if (UNIX)
  # show quite some warnings (but remove some intentionally)
  include(CheckCXXCompilerFlag)
  add_compile_options(-Wall -Wextra -pedantic)
  foreach (WARNING unknown-pragmas)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()
  if (ENABLE_COVERAGE)
    add_compile_options(-O0 -g --coverage -fprofile-arcs -ftest-coverage)
  endif()
elseif (WIN32)
  add_compile_definitions(NOMINMAX=1)
endif()

add_subdirectory(include)
add_subdirectory(lib)

if(LORINA_TEST)
  add_subdirectory(test)
endif()
