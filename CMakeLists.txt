# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.5.1)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message(STATUS "ccache enabled (${CCACHE_PROGRAM})")
endif()

PROJECT(iroha C CXX)

SET(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
SET(CMAKE_CXX_FLAGS "-std=c++14 -Wall -fdiagnostics-color=always")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-error=deprecated-declarations")
SET(CMAKE_CXX_FLAGS_DEBUG   "-g -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -O0")
SET(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
SET(CMAKE_INSTALL_RPATH "../lib")

if(COVERAGE)
  find_program(LCOV_PROGRAM lcov)
  if(NOT LCOV_PROGRAM)
    message(FATAL_ERROR "lcov not found! Aborting...")
  endif()
  find_file(LCOV_CONFIG_FILE .lcovrc ${PROJECT_SOURCE_DIR})
  if(NOT LCOV_CONFIG_FILE)
    message(FATAL_ERROR "lcov config file not found in project root! Aborting...")
  endif()
  message(STATUS "lcov enabled (${LCOV_PROGRAM})")
  # remove -g flag to reduce binary size
  list(REMOVE_ITEM CMAKE_CXX_FLAGS -g)
  set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
  set(CMAKE_CXX_FLAGS "--coverage ${CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "--coverage ${CMAKE_C_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS "--coverage ${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "--coverage ${CMAKE_EXE_LINKER_FLAGS}")
  add_custom_target(coverage.init.info
      COMMAND ${LCOV_PROGRAM} --config-file ${LCOV_CONFIG_FILE} -o ${PROJECT_BINARY_DIR}/reports/coverage.init.info -c -i -d ${PROJECT_BINARY_DIR}
      )
  add_custom_target(coverage.info
      COMMAND ${LCOV_PROGRAM} --config-file ${LCOV_CONFIG_FILE} -o ${PROJECT_BINARY_DIR}/reports/coverage.info -c -d ${PROJECT_BINARY_DIR}
      COMMAND ${LCOV_PROGRAM} --config-file ${LCOV_CONFIG_FILE} -o ${PROJECT_BINARY_DIR}/reports/coverage.info -a ${PROJECT_BINARY_DIR}/reports/coverage.init.info -a ${PROJECT_BINARY_DIR}/reports/coverage.info
      COMMAND ${LCOV_PROGRAM} --config-file ${LCOV_CONFIG_FILE} -o ${PROJECT_BINARY_DIR}/reports/coverage.info -r ${PROJECT_BINARY_DIR}/reports/coverage.info '/usr*' '${CMAKE_SOURCE_DIR}/external/*' '${CMAKE_SOURCE_DIR}/schema/*'
      )
  set(REPORT_DIR ${CMAKE_BINARY_DIR}/reports)
  file(MAKE_DIRECTORY ${REPORT_DIR})
  include(cmake/analysis.cmake)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

option(BENCHMARKING "Build benchmarks"               OFF)
option(TESTING      "Build tests"                    ON )
option(COVERAGE     "Enable coverage"                OFF)
option(FUZZING      "Build fuzzing binaries"         OFF)
option(PACKAGE_ZIP  "Create ZIP package"             OFF)
option(PACKAGE_TGZ  "Create TGZ package"             OFF)
option(PACKAGE_RPM  "Create RPM package"             OFF)
option(PACKAGE_DEB  "Create DEB package"             OFF)
option(ENABLE_LIBS_PACKAGING "Enable libs packaging" ON)
option(SWIG_PYTHON  "Generate Swig Python bindings"  OFF)
option(SWIG_JAVA    "Generate Swig Java bindings"    OFF)
option(SUPPORT_PYTHON2 "ON if Python2, OFF if python3" OFF)
option(SWIG_CSHARP  "Generate Swig C# bindings"      OFF)
option(SWIG_NODE    "Generate Swig NodeJS"           OFF)

SET(BENCHMARKING ON)

if (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_GENERATOR MATCHES "Make")
  set(MAKE "$(MAKE)")
else()
  set(MAKE make)
endif()

if(PACKAGE_TGZ OR PACKAGE_ZIP OR PACKAGE_RPM OR PACKAGE_DEB)
  message(STATUS "Packaging enabled: forcing non-packaging options to OFF")
  set(BENCHMARKING OFF)
  set(TESTING OFF)
  set(COVERAGE OFF)
  set(FUZZING OFF)
  set(SWIG_PYTHON OFF)
  set(SWIG_JAVA OFF)
  set(SWIG_NODE OFF)
endif()

message(STATUS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "-DTESTING=${TESTING}")
message(STATUS "-DBENCHMARKING=${BENCHMARKING}")
message(STATUS "-DFUZZING=${FUZZING}")
message(STATUS "-DCOVERAGE=${COVERAGE}")
message(STATUS "-DPACKAGE_ZIP=${PACKAGE_ZIP}")
message(STATUS "-DPACKAGE_TGZ=${PACKAGE_TGZ}")
message(STATUS "-DPACKAGE_RPM=${PACKAGE_RPM}")
message(STATUS "-DPACKAGE_DEB=${PACKAGE_DEB}")
message(STATUS "-DENABLE_LIBS_PACKAGING=${ENABLE_LIBS_PACKAGING}")
message(STATUS "-DSWIG_PYTHON=${SWIG_PYTHON}")
message(STATUS "-DSWIG_JAVA=${SWIG_JAVA}")
message(STATUS "-DSUPPORT_PYTHON2=${SUPPORT_PYTHON2}")
message(STATUS "-DSWIG_CSHARP=${SWIG_CSHARP}")
message(STATUS "-DSWIG_NODE=${SWIG_NODE}")

set(IROHA_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/schema")
set(SM_SCHEMA_DIR "${PROJECT_SOURCE_DIR}/shared_model/schema")
set(SCHEMA_OUT_DIR ${CMAKE_BINARY_DIR}/schema)
include_directories(
  ${PROJECT_SOURCE_DIR}/irohad
  ${PROJECT_SOURCE_DIR}/shared_model
  ${PROJECT_SOURCE_DIR}/libs
)

SET(IROHA_ROOT_PROJECT ON)

# Boost uses RTTI to perform some actions (such as type erasure).
# This is slow. This flag forces boost to use other methods,
# which are generally faster
add_definitions(-DBOOST_NO_RTTI)

include(FeatureSummary)
include(cmake/functions.cmake)
include(cmake/dependencies.cmake)
include(cmake/clang-cxx-dev-tools.cmake)

if(PACKAGE_TGZ OR PACKAGE_ZIP OR PACKAGE_RPM OR PACKAGE_DEB)
  include(cmake/release.cmake)
endif()

add_subdirectory(schema)
add_subdirectory(libs)
add_subdirectory(irohad)
add_subdirectory(iroha-cli)
add_subdirectory(shared_model)

if(TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

if (FUZZING)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(Fuzzing with compilers other than clang is not supported yet)
  endif()
endif()
