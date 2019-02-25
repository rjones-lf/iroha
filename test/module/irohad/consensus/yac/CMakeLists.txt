# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# Testing of transaction processor
addtest(cluster_order_test cluster_order_test.cpp)
target_link_libraries(cluster_order_test
    yac
    )

addtest(yac_cold_case_test yac_simple_cold_case_test.cpp)
target_link_libraries(yac_cold_case_test
    yac
    )

addtest(yac_sunny_day_test yac_sunny_day_test.cpp)
target_link_libraries(yac_sunny_day_test
    yac
    )

addtest(yac_rainy_day_test yac_rainy_day_test.cpp)
target_link_libraries(yac_rainy_day_test
    yac
    )

addtest(yac_unknown_peer_test yac_unknown_peer_test.cpp)
target_link_libraries(yac_unknown_peer_test
    yac
    )

addtest(yac_block_storage_test yac_block_storage_test.cpp)
target_link_libraries(yac_block_storage_test
    yac
    )

addtest(yac_proposal_storage_test yac_proposal_storage_test.cpp)
target_link_libraries(yac_proposal_storage_test
    yac
    )

addtest(yac_timer_test timer_test.cpp)
target_link_libraries(yac_timer_test
    yac
    )

addtest(yac_network_test network_test.cpp)
target_link_libraries(yac_network_test
    yac
    yac_transport
    )

addtest(yac_peer_orderer_test peer_orderer_test.cpp)
target_link_libraries(yac_peer_orderer_test
    yac
    )

addtest(yac_gate_test yac_gate_test.cpp)
target_link_libraries(yac_gate_test
    yac
    )

addtest(yac_hash_provider_test yac_hash_provider_test.cpp)
target_link_libraries(yac_hash_provider_test
    yac
    )

addtest(yac_common_test yac_common_test.cpp)
target_link_libraries(yac_common_test
    yac
    )

addtest(yac_crypto_provider_test yac_crypto_provider_test.cpp)
target_link_libraries(yac_crypto_provider_test
    yac
    yac_transport
    )

addtest(supermajority_checker_test supermajority_checker_test.cpp)
target_link_libraries(supermajority_checker_test
    yac
    )
