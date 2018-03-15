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

add_subdirectory(generators)
add_subdirectory(converters)

add_library(common_execution
    execution/impl/common_executor.cpp
    )

target_link_libraries(common_execution
    ametsuchi
    common
    )

add_library(command_execution
    execution/impl/command_executor.cpp
    execution/impl/command_executor_factory.cpp
    )
target_link_libraries(command_execution
    common_execution
    validator
    boost
    )

add_library(sha3_hash
    sha3_hash.cpp
    )

target_link_libraries(sha3_hash
    pb_model_converters
    )

add_library(model
    model_crypto_provider_impl.cpp
    impl/model_operators.cpp
    impl/query_execution.cpp
    )
target_link_libraries(model
    hash
    sha3_hash
    optional
    rxcpp
    logger
    iroha_amount
    common_execution
    schema
    ed25519_crypto
    rapidjson
    )

add_library(model_registrations INTERFACE)
target_include_directories(model_registrations INTERFACE
    registration
    )
target_link_libraries(model_registrations INTERFACE
    model
    )
