# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

addtest(ordering_service_test ordering_service_test.cpp)
target_link_libraries(ordering_service_test
    ordering_service
    shared_model_stateless_validation
    shared_model_proto_backend
    )

addtest(ordering_gate_test ordering_gate_test.cpp)
target_link_libraries(ordering_gate_test
    ordering_service
    shared_model_cryptography_model
    shared_model_stateless_validation
    consensus_round
    )

addtest(on_demand_os_test on_demand_os_test.cpp)
target_link_libraries(on_demand_os_test
    on_demand_ordering_service
    shared_model_default_builders
    )

addtest(on_demand_os_client_grpc_test on_demand_os_client_grpc_test.cpp)
target_link_libraries(on_demand_os_client_grpc_test
    on_demand_ordering_service_transport_grpc
    )

addtest(on_demand_os_server_grpc_test on_demand_os_server_grpc_test.cpp)
target_link_libraries(on_demand_os_server_grpc_test
    on_demand_ordering_service_transport_grpc
    )

addtest(on_demand_connection_manager_test on_demand_connection_manager_test.cpp)
target_link_libraries(on_demand_connection_manager_test
    on_demand_connection_manager
    )

addtest(on_demand_ordering_gate_test on_demand_ordering_gate_test.cpp)
target_link_libraries(on_demand_ordering_gate_test
    on_demand_ordering_gate
    )
