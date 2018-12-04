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
    )

addtest(on_demand_os_test on_demand_os_test.cpp)
target_link_libraries(on_demand_os_test
    on_demand_ordering_service
    shared_model_default_builders
    ametsuchi
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
    shared_model_interfaces_factories
    )

addtest(on_demand_cache_test on_demand_cache_test.cpp)
target_link_libraries(on_demand_cache_test
    on_demand_ordering_gate
    shared_model_interfaces_factories
    )
