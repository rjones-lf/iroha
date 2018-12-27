#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_subdirectory(processor)

# command service test
addtest(torii_service_test torii_service_test.cpp)
target_link_libraries(torii_service_test
    torii_service
    command_client
    query_client
    server_runner
    processors
    consensus_round
    on_demand_common
    )

addtest(torii_queries_test torii_queries_test.cpp)
target_link_libraries(torii_queries_test
    torii_service
    command_client
    query_client
    server_runner
    processors
    )

addtest(query_service_test query_service_test.cpp)
target_link_libraries(query_service_test
    torii_service
    query_client
    )

addtest(torii_service_query_test torii_service_query_test.cpp)
target_link_libraries(torii_service_query_test
    torii_service
    server_runner
    query_client
    )
