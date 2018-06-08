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

add_library(shared_model_interfaces
    impl/permissions.cpp
    commands/impl/add_asset_quantity.cpp
    commands/impl/add_peer.cpp
    commands/impl/add_signatory.cpp
    commands/impl/append_role.cpp
    commands/impl/command.cpp
    commands/impl/create_account.cpp
    commands/impl/create_asset.cpp
    commands/impl/create_domain.cpp
    commands/impl/create_role.cpp
    commands/impl/detach_role.cpp
    commands/impl/grant_permission.cpp
    commands/impl/remove_signatory.cpp
    commands/impl/revoke_permission.cpp
    commands/impl/set_account_detail.cpp
    commands/impl/set_quorum.cpp
    commands/impl/subtract_asset_quantity.cpp
    commands/impl/transfer_asset.cpp
    query_responses/impl/block_error_response.cpp
    query_responses/impl/block_query_response.cpp
    query_responses/impl/block_response.cpp
    iroha_internal/block_variant.cpp
    )

target_link_libraries(shared_model_interfaces
    shared_model_cryptography
    ${Boost_LIBRARIES}
    )
