# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=fuzzer,address")

find_package(protobuf-mutator REQUIRED)

add_executable(torii_fuzzing torii_fuzz.cpp)
target_link_libraries(torii_fuzzing
  rxcpp
  gtest::gtest
  gmock::gmock
  ametsuchi
  libs_common
  torii_service
  protobuf-mutator
  )

add_executable(status_fuzzing status_fuzz.cpp)
target_link_libraries(status_fuzzing
  rxcpp
  gtest::gtest
  gmock::gmock
  ametsuchi
  libs_common
  torii_service
  protobuf-mutator
  )

add_executable(find_fuzzing find_fuzz.cpp)
target_link_libraries(find_fuzzing
  rxcpp
  gtest::gtest
  gmock::gmock
  ametsuchi
  libs_common
  torii_service
  protobuf-mutator
  )
