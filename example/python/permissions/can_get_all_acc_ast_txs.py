#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
import commons

admin = commons.new_user('admin@first')
alice = commons.new_user('alice@second')


@commons.hex
def genesis_tx():
    test_permissions = iroha.RolePermissionSet([
        iroha.Role_kGetAllAccAstTxs,
        iroha.Role_kReceive,
        iroha.Role_kTransfer
    ])
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(admin['id']) \
        .addPeer('0.0.0.0:50541', admin['key'].publicKey()) \
        .createRole('admin_role', commons.all_permissions()) \
        .createRole('test_role', test_permissions) \
        .createDomain('first', 'admin_role') \
        .createDomain('second', 'test_role') \
        .createAccount('admin', 'first', admin['key'].publicKey()) \
        .createAccount('alice', 'second', alice['key'].publicKey()) \
        .createAsset('coin', 'first', 2) \
        .addAssetQuantity('coin#first', '300.00') \
        .transferAsset(admin['id'], alice['id'], 'coin#first', 'top up', '200.00') \
        .build()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(admin['key']).finish()


@commons.hex
def account_asset_transactions_query():
    tx = iroha.ModelQueryBuilder() \
        .createdTime(commons.now()) \
        .queryCounter(1) \
        .creatorAccountId(alice['id']) \
        .getAccountAssetTransactions(admin['id'], 'coin#first') \
        .build()
    return iroha.ModelProtoQuery(tx) \
        .signAndAddSignature(alice['key']).finish()
