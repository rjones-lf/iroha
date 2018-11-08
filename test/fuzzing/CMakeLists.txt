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
  torii_service
  protobuf-mutator
  )

add_executable(status_fuzzing status_fuzz.cpp)
target_link_libraries(status_fuzzing
  rxcpp
  gtest::gtest
  gmock::gmock
  ametsuchi
  torii_service
  protobuf-mutator
  )

add_executable(find_fuzzing find_fuzz.cpp)
target_link_libraries(find_fuzzing
  rxcpp
  gtest::gtest
  gmock::gmock
  ametsuchi
  torii_service
  protobuf-mutator
  )

add_executable(ordering_gate_fuzz ordering_gate_fuzz.cpp)
target_link_libraries(ordering_gate_fuzz
  rxcpp
  gtest::gtest
  gmock::gmock
  on_demand_ordering_gate
  shared_model_default_builders
  shared_model_stateless_validation
  protobuf-mutator
)
