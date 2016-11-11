cmake_minimum_required(VERSION 3.0)
PROJECT(iroha CXX)

ENABLE_LANGUAGE(C)
find_package(Threads)

SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++1y")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wextra -O0")
SET(CMAKE_SHARED_LINKER_FLAGS "-lpthread -lssl")

add_subdirectory(core)
add_subdirectory(peer)

if(NOT without-test)
  enable_testing()
  include(test/gtest.cmake)
  add_subdirectory(test)
endif()