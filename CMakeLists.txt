cmake_minimum_required(VERSION 2.8)
PROJECT(iroha C CXX)

SET(CMAKE_CXX_FLAGS "-g -std=c++1y -Wall -fPIC")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wextra -Wno-unused-parameter -O0")
#SET(CMAKE_SHARED_LINKER_FLAGS "-lpthread -lssl")
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

option(BENCHMARKING "Build benchmarks" OFF)
option(TESTING "Build tests" ON)

# set default CMAKE_BUILD_TYPE=Debug
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "-DTESTING=${TESTING}")
message(STATUS "-DBENCHMARKING=${BENCHMARKING}")

include_directories(${PROJECT_SOURCE_DIR}/core)
include("dependencies.cmake")

add_subdirectory(protos)

add_subdirectory(core)
add_subdirectory(peer)
add_subdirectory(tools)
add_subdirectory(smart_contract)

if(TESTING)
  enable_testing()
  add_subdirectory(test)
endif()


if(BENCHMARKING)
  add_subdirectory(benchmark)
endif()
