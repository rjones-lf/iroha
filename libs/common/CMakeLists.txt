# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(common INTERFACE
  # bind.hpp
  # blob.hpp
  # byteutils.hpp
  # cloneable.hpp
  # default_constructible_unary_fn.hpp
  # instanceof.hpp
  # is_any.hpp
  # obj_utils.hpp
  # result.hpp
  # set.hpp
  # visitor.hpp
  )
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

add_library(libs_timeout INTERFACE
  # timeout.hpp
  )
target_link_libraries(libs_timeout INTERFACE
  common
  rxcpp
  )
