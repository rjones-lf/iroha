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

addtest(transport_builder_test
    transport_builder_test.cpp
    )
target_link_libraries(transport_builder_test
    shared_model_proto_builders
    shared_model_stateless_validation
    )

addtest(proto_peer_builder_test
    common_objects/proto_peer_builder_test.cpp
    )

target_link_libraries(proto_peer_builder_test
    shared_model_proto_builders
    )

addtest(proto_account_builder_test
    common_objects/proto_account_builder_test.cpp
    )

target_link_libraries(proto_account_builder_test
    shared_model_proto_builders
    )

addtest(proto_amount_builder_test
    common_objects/proto_amount_builder_test.cpp
    )

target_link_libraries(proto_amount_builder_test
    shared_model_proto_builders
    )

addtest(proto_signature_builder_test
    common_objects/proto_signature_builder_test.cpp
    )

target_link_libraries(proto_signature_builder_test
    shared_model_proto_builders
    )

addtest(proto_asset_builder_test
    common_objects/proto_asset_builder_test.cpp
    )

target_link_libraries(proto_asset_builder_test
    shared_model_proto_builders
    )

addtest(proto_account_asset_builder_test
    common_objects/proto_account_asset_builder_test.cpp
    )

target_link_libraries(proto_account_asset_builder_test
    shared_model_proto_builders
    )

addtest(proto_transaction_responses_builder_test
    transaction_responses/proto_transaction_response_builder.cpp)

target_link_libraries(proto_transaction_responses_builder_test
    shared_model_proto_builders
    )
