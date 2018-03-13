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

add_library(json_model_converters
    impl/json_block_factory.cpp
    impl/json_command_factory.cpp
    impl/json_common.cpp
    impl/json_query_factory.cpp
    impl/json_transaction_factory.cpp
    )
target_link_libraries(json_model_converters
    model
    rapidjson
    schema
    ed25519_crypto
    logger
    )

add_library(pb_model_converters
    impl/pb_block_factory.cpp
    impl/pb_command_factory.cpp
    impl/pb_query_factory.cpp
    impl/pb_query_response_factory.cpp
    impl/pb_transaction_factory.cpp
    impl/pb_common.cpp
    )
target_link_libraries(pb_model_converters
    model
    schema
    ed25519_crypto
    logger
    boost
    )
