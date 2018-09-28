# Copyright 2017 Soramitsu Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

AddTest(state_test state_test.cpp)
target_link_libraries(state_test
    mst_state
    logger
    shared_model_proto_builders
    shared_model_stateless_validation
    )

AddTest(storage_test storage_test.cpp)
target_link_libraries(storage_test
    mst_storage
    logger
    shared_model_proto_builders
    shared_model_stateless_validation
    )

AddTest(transport_test transport_test.cpp)
target_link_libraries(transport_test
    mst_transport
    mst_processor
    logger
    shared_model_cryptography
    shared_model_stateless_validation
    shared_model_proto_backend
    )

AddTest(mst_processor_test mst_processor_test.cpp)
target_link_libraries(mst_processor_test
    mst_processor
    logger
    shared_model_proto_builders
    shared_model_stateless_validation
    )

AddTest(gossip_propagation_strategy_test
    gossip_propagation_strategy_test.cpp
    )
target_link_libraries(gossip_propagation_strategy_test
    mst_processor
    logger
    shared_model_cryptography
    shared_model_stateless_validation
    )
