#
# Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include_directories(
  # Library headers.
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${IROHA_SCHEMA_DIR}"
)

include(${PROJECT_SOURCE_DIR}/cmake/functions.cmake)

set(AMETSUCHI_INCLUDE
  include/ametsuchi/ametsuchi.h
  include/ametsuchi/tx_store.h
  include/ametsuchi/wsv.h
  include/ametsuchi/common.h
  include/ametsuchi/currency.h
  include/ametsuchi/exception.h
  include/ametsuchi/comparator.h
  include/ametsuchi/merkle_tree/narrow_merkle_tree.h
  include/ametsuchi/merkle_tree/circular_stack.h
  include/ametsuchi/merkle_tree/merkle_tree.h

  # needed to compile fbs automatically
  #${IROHA_SCHEMA_DIR}/account_generated.h
  #${IROHA_SCHEMA_DIR}/asset_generated.h
  #${IROHA_SCHEMA_DIR}/commands_generated.h
  #${IROHA_SCHEMA_DIR}/main_generated.h
  #${IROHA_SCHEMA_DIR}/primitives_generated.h
  #${IROHA_SCHEMA_DIR}/transaction_generated.h
  )

set(AMETSUCHI_SRC
  src/ametsuchi/ametsuchi.cc
  src/ametsuchi/tx_store.cc
  src/ametsuchi/wsv.cc
  src/ametsuchi/currency.cc
  src/ametsuchi/common.cc
  src/ametsuchi/merkle_tree/merkle_tree.cc
)

# Library.
set(LIBAMETSUCHI_NAME ametsuchi)
add_library(${LIBAMETSUCHI_NAME} SHARED
  ${AMETSUCHI_INCLUDE}
  ${AMETSUCHI_SRC}
)
target_link_libraries(${LIBAMETSUCHI_NAME}
  spdlog
  LMDB
  flatbuffers
  keccak
)

StrictMode(${LIBAMETSUCHI_NAME})





