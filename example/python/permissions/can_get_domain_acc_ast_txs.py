#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
import commons

admin = commons.new_user('admin@test')
alice = commons.new_user('alice@test')


@commons.hex
def genesis_tx():
    test_permissions = iroha.RolePermissionSet([iroha.Role_kGetDomainAccAstTxs])
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(admin['id']) \
        .addPeer('0.0.0.0:50541', admin['key'].publicKey()) \
        .createRole('admin_role', commons.all_permissions()) \
        .createRole('test_role', test_permissions) \
        .createDomain('test', 'test_role') \
        .createAccount('admin', 'test', admin['key'].publicKey()) \
        .createAccount('alice', 'test', alice['key'].publicKey()) \
        .createAsset('coin', 'test', 2) \
        .addAssetQuantity('coin#test', '500.69') \
        .transferAsset(admin['id'], alice['id'], 'coin#test', 'top up', '10.00') \
        .build()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(admin['key']).finish()


@commons.hex
def account_asset_transactions_query():
    tx = iroha.ModelQueryBuilder() \
        .createdTime(commons.now()) \
        .queryCounter(1) \
        .creatorAccountId(alice['id']) \
        .getAccountAssetTransactions(admin['id'], 'coin#test') \
        .build()
    return iroha.ModelProtoQuery(tx) \
        .signAndAddSignature(alice['key']).finish()
