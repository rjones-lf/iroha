# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(common INTERFACE)
target_link_libraries(common INTERFACE
  boost
  )

add_library(libs_files
  files.cpp
  )
target_link_libraries(libs_files
  logger
  common
  )

add_library(libs_timeout INTERFACE)
target_link_libraries(libs_timeout INTERFACE
  common
  rxcpp
  )
