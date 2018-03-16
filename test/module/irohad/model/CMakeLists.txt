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

addtest(model_crypto_provider_test model_crypto_provider_test.cpp)
target_link_libraries(model_crypto_provider_test
    model
    hash
    ed25519_crypto
    model_generators
    )

addtest(command_converter_test converters/pb_commands_test.cpp)
target_link_libraries(command_converter_test
    pb_model_converters
    )

addtest(transaction_converter_test converters/pb_transaction_test.cpp)
target_link_libraries(transaction_converter_test
    pb_model_converters
    )

addtest(block_converter_test converters/pb_block_test.cpp)
target_link_libraries(block_converter_test
    pb_model_converters
    )

addtest(query_responses_test converters/pb_query_responses_test.cpp)
target_link_libraries(query_responses_test
    pb_model_converters
    )

addtest(model_operators_test operators/model_operators_test.cpp)
target_link_libraries(model_operators_test
    model
    )

addtest(command_validate_execute_test command_validate_execute_test.cpp)
target_link_libraries(command_validate_execute_test
    model
    command_execution
    )

addtest(json_command_converter_test converters/json_commands_test.cpp)
target_link_libraries(json_command_converter_test
    json_model_converters
    )

addtest(json_transaction_converter_test converters/json_transaction_test.cpp)
target_link_libraries(json_transaction_converter_test
    json_model_converters
    )

addtest(json_block_converter_test converters/json_block_test.cpp)
target_link_libraries(json_block_converter_test
    json_model_converters
    )

addtest(json_query_factory_test converters/json_query_factory_test.cpp)
target_link_libraries(json_query_factory_test
    model_generators
    json_model_converters
    )

addtest(pb_query_factory_test converters/pb_query_factory_test.cpp)
target_link_libraries(pb_query_factory_test
    model_generators
    pb_model_converters
    )

addtest(static_map static_map.cpp)
target_link_libraries(static_map
    model_registrations
    logger
    )
