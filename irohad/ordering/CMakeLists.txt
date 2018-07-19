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

add_library(ordering_service
    impl/ordering_gate_impl.cpp
    impl/ordering_service_impl.cpp
    impl/ordering_gate_transport_grpc.cpp
    impl/ordering_service_transport_grpc.cpp
    )


target_link_libraries(ordering_service
    rxcpp
    tbb
    shared_model_interfaces
    ordering_grpc
    logger
    )
