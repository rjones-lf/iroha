#
# Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
# http://soramitsu.co.jp
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_library(server_runner server_runner.cpp)
target_link_libraries(server_runner
    logger
    grpc++
    boost # iroha::expected::Result
    )

add_library(raw_block_loader impl/raw_block_loader.cpp)
target_link_libraries(raw_block_loader
    shared_model_interfaces
    )

add_library(application
    application.cpp
    impl/ordering_init.cpp
    impl/consensus_init.cpp
    impl/block_loader_init.cpp
    )
target_link_libraries(application
    logger
    yac
    server_runner
    ametsuchi
    networking
    ordering_service
    chain_validator
    hash
    stateful_validator
    processors
    ed25519_crypto
    simulator
    block_loader
    block_loader_service
    torii_service
    )

add_executable(irohad irohad.cpp)
target_link_libraries(irohad
    application
    raw_block_loader
    gflags
    rapidjson
    keys_manager
    )

add_install_step_for_bin(irohad)
