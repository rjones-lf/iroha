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

# Testing of transaction processor
addtest(cluster_order_test cluster_order_test.cpp)
target_link_libraries(cluster_order_test
    yac
    shared_model_cryptography_model
    )

addtest(yac_cold_case_test yac_simple_cold_case_test.cpp)
target_link_libraries(yac_cold_case_test
    yac
    model
    shared_model_cryptography_model
    )

addtest(yac_sunny_day_test yac_sunny_day_test.cpp)
target_link_libraries(yac_sunny_day_test
    yac
    model
    shared_model_cryptography_model
    )

addtest(yac_rainy_day_test yac_rainy_day_test.cpp)
target_link_libraries(yac_rainy_day_test
    yac
    model
    shared_model_cryptography_model
    )

addtest(yac_unknown_peer_test yac_unknown_peer_test.cpp)
target_link_libraries(yac_unknown_peer_test
    yac
    model
    shared_model_stateless_validation
    )

addtest(yac_block_storage_test yac_block_storage_test.cpp)
target_link_libraries(yac_block_storage_test
    yac
    model
    shared_model_cryptography_model
    )

addtest(yac_proposal_storage_test yac_proposal_storage_test.cpp)
target_link_libraries(yac_proposal_storage_test
    yac
    model
    shared_model_stateless_validation
    )

addtest(yac_timer_test timer_test.cpp)
target_link_libraries(yac_timer_test
    yac
    )

addtest(yac_network_test network_test.cpp)
target_link_libraries(yac_network_test
    yac
    model
    shared_model_stateless_validation
    )

addtest(yac_peer_orderer_test peer_orderer_test.cpp)
target_link_libraries(yac_peer_orderer_test
    yac
    shared_model_stateless_validation
    model
    )

addtest(yac_gate_test yac_gate_test.cpp)
target_link_libraries(yac_gate_test
    yac
    model
    shared_model_cryptography_model
    shared_model_stateless_validation
    )

addtest(yac_hash_provider_test yac_hash_provider_test.cpp)
target_link_libraries(yac_hash_provider_test
    yac
    shared_model_cryptography_model
    )

addtest(yac_common_test yac_common_test.cpp)
target_link_libraries(yac_common_test
    yac
    shared_model_cryptography_model
    )

addtest(yac_crypto_provider_test yac_crypto_provider_test.cpp)
target_link_libraries(yac_crypto_provider_test
    yac
    model
    shared_model_cryptography_model
    shared_model_stateless_validation
    )

addtest(supermajority_checker_test supermajority_checker_test.cpp)
target_link_libraries(supermajority_checker_test
    yac
    model
    )
