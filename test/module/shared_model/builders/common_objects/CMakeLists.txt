# Copyright 2018 Soramitsu Co., Ltd.
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

addtest(peer_builder_test
        peer_builder_test.cpp
        )

target_link_libraries(peer_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

addtest(account_builder_test
        account_builder_test.cpp
        )

target_link_libraries(account_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

addtest(amount_builder_test
        amount_builder_test.cpp
        )

target_link_libraries(amount_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

addtest(signature_builder_test
        signature_builder_test.cpp
        )

target_link_libraries(signature_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

addtest(asset_builder_test
        asset_builder_test.cpp
        )

target_link_libraries(asset_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

addtest(account_asset_builder_test
        account_asset_builder_test.cpp
        )

target_link_libraries(account_asset_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

addtest(query_response_builder_test
        query_response_builder_test.cpp
        )

target_link_libraries(query_response_builder_test
        shared_model_proto_builders
        )
