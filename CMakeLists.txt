cmake_minimum_required(VERSION 3.0)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message(STATUS "ccache enabled (${CCACHE_PROGRAM})")
endif()

PROJECT(iroha C CXX)

SET(CMAKE_CXX_FLAGS "-std=c++1y -Wall -fPIC")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG   "-g -Wextra -Wno-unused-parameter -O0")
SET(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

if(COVERAGE)
  set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
  set(CMAKE_CXX_FLAGS "--coverage ${CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "--coverage ${CMAKE_C_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS "--coverage ${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "--coverage ${CMAKE_EXE_LINKER_FLAGS}")

  set(REPORT_DIR ${CMAKE_BINARY_DIR}/reports)
  file(MAKE_DIRECTORY ${REPORT_DIR})
  include(cmake/analysis.cmake)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

option(BENCHMARKING "Build benchmarks" OFF)
option(TESTING "Build tests" ON)
option(COVERAGE "Enable coverage" OFF)
option(FUZZING "Build fuzzing binaries" OFF)

if (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "-DTESTING=${TESTING}")
message(STATUS "-DBENCHMARKING=${BENCHMARKING}")
message(STATUS "-DFUZZING=${FUZZING}")
message(STATUS "-DCOVERAGE=${COVERAGE}")

SET(IROHA_SCHEMA_DIR "${PROJECT_SOURCE_DIR}/schema")
include_directories(
  ${PROJECT_SOURCE_DIR}/irohad
  ${PROJECT_SOURCE_DIR}/libs
  ${PROJECT_SOURCE_DIR}/libs/gsl/include
  ${IROHA_SCHEMA_DIR}
)

include(cmake/dependencies.cmake)
include(cmake/functions.cmake)
add_subdirectory(schema)
add_subdirectory(libs)
add_subdirectory(irohad)
add_subdirectory(iroha-cli)


link_directories(${PROJECT_BINARY_DIR}/lib)

if(TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

if(BENCHMARKING)
  add_subdirectory(benchmark)
endif()

if (FUZZING)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_subdirectory(fuzz)
  else()
    message(Fuzzing with compilers other than clang does not supported yet)
  endif()
endif()
