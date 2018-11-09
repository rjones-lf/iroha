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
    boost
    common
    )

add_library(raw_block_loader impl/raw_block_loader.cpp)
target_link_libraries(raw_block_loader
    shared_model_interfaces
    shared_model_proto_backend
    shared_model_stateless_validation
    logger
    )

add_library(application
    application.cpp
    # TODO andrei 08.11.2018 IR-1851 Create separate targets for initialization
    # helpers in application
    impl/on_demand_ordering_init.cpp
    impl/consensus_init.cpp
    impl/block_loader_init.cpp
    )
target_link_libraries(application
    logger
    yac
    yac_transport
    server_runner
    ametsuchi
    networking
    on_demand_ordering_service
    on_demand_ordering_service_transport_grpc
    on_demand_connection_manager
    on_demand_ordering_gate
    on_demand_common
    chain_validator
    stateful_validator
    processors
    ed25519_crypto
    simulator
    block_loader
    block_loader_service
    mst_processor
    torii_service
    pending_txs_storage
    common
    )

add_executable(irohad irohad.cpp)
target_link_libraries(irohad
    application
    raw_block_loader
    gflags
    rapidjson
    keys_manager
    common
    )

add_install_step_for_bin(irohad)
