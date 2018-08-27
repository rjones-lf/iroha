#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(acceptance_fixture acceptance_fixture.cpp)
target_link_libraries(acceptance_fixture
    gtest::gtest
    gmock::gmock
    integration_framework
    shared_model_proto_builders
    )

add_library(grantable_permissions_fixture
    grantable_permissions_fixture.cpp
    )
target_link_libraries(grantable_permissions_fixture
    acceptance_fixture
    )

addtest(add_asset_qty_test add_asset_qty_test.cpp)
target_link_libraries(add_asset_qty_test
    acceptance_fixture
    )

addtest(create_account_test create_account_test.cpp)
target_link_libraries(create_account_test
    acceptance_fixture
    )

addtest(create_domain_test create_domain_test.cpp)
target_link_libraries(create_domain_test
    acceptance_fixture
    )

addtest(create_role_test create_role_test.cpp)
target_link_libraries(create_role_test
    acceptance_fixture
    )

addtest(get_transactions_test get_transactions_test.cpp)
target_link_libraries(get_transactions_test
    acceptance_fixture
    )

addtest(grant_permission_test grant_permission_test.cpp)
target_link_libraries(grant_permission_test
    grantable_permissions_fixture
    )

addtest(revoke_permission_test revoke_permission_test.cpp)
target_link_libraries(revoke_permission_test
    grantable_permissions_fixture
    )

addtest(invalid_fields_test invalid_fields_test.cpp)
target_link_libraries(invalid_fields_test
    acceptance_fixture
    )

addtest(query_test query_test.cpp)
target_link_libraries(query_test
    acceptance_fixture
    )

addtest(subtract_asset_qty_test subtract_asset_qty_test.cpp)
target_link_libraries(subtract_asset_qty_test
    acceptance_fixture
    )

addtest(transfer_asset_test transfer_asset_test.cpp)
target_link_libraries(transfer_asset_test
    acceptance_fixture
    )

addtest(tx_acceptance_test tx_acceptance_test.cpp)
target_link_libraries(tx_acceptance_test
    acceptance_fixture
    )

addtest(tx_heavy_data tx_heavy_data.cpp)
target_link_libraries(tx_heavy_data
    acceptance_fixture
    )

addtest(get_account_assets_test get_account_assets_test.cpp)
target_link_libraries(get_account_assets_test
    acceptance_fixture
    )

addtest(set_account_detail_test set_account_detail_test.cpp)
target_link_libraries(set_account_detail_test
    acceptance_fixture
    )

addtest(get_roles_test get_roles_test.cpp)
target_link_libraries(get_roles_test
    acceptance_fixture
    )

addtest(create_asset_test create_asset_test.cpp)
target_link_libraries(create_asset_test
    acceptance_fixture
    )

addtest(get_account_asset_txs_test
    get_account_asset_txs_test.cpp
    )
target_link_libraries(get_account_asset_txs_test
    acceptance_fixture
    )

addtest(get_account_test
    get_account_test.cpp
    )
target_link_libraries(get_account_test
    acceptance_fixture
    )
