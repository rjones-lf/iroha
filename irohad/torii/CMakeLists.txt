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

add_subdirectory(processor)

add_library(query_client STATIC query_client.cpp)

add_library(command_client command_client.cpp)
target_link_libraries(command_client
    torii_service
    endpoint
    schema
    )
target_link_libraries(query_client
    torii_service
    endpoint
    )

add_library(torii_service
    impl/query_service.cpp
    impl/command_service.cpp
    )
target_link_libraries(torii_service
    endpoint
    shared_model_proto_backend
    logger
    shared_model_stateless_validation
    )
