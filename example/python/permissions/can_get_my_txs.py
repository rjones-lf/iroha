#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
import commons

admin = commons.new_user('admin@first')
alice = commons.new_user('alice@second')

alice_tx1_hash = None
alice_tx2_hash_blob = None


@commons.hex
def genesis_tx():
    test_permissions = iroha.RolePermissionSet([
        iroha.Role_kGetMyTxs,
        iroha.Role_kAddAssetQty,
        iroha.Role_kCreateAsset
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
        .build()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(admin['key']).finish()


@commons.hex
def alice_action_1_tx():
    global alice_tx1_hash
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(alice['id']) \
        .createAsset('coin', 'first', 2) \
        .build()
    alice_tx1_hash = tx.hash()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(alice['key']).finish()


@commons.hex
def alice_action_2_tx():
    global alice_tx2_hash_blob
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(alice['id']) \
        .addAssetQuantity('coin#first', '600.30') \
        .build()
    alice_tx2_hash_blob = tx.hash().blob()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(alice['key']).finish()


@commons.hex
def transactions_query():
    hashes = iroha.HashVector()
    hashes.append(alice_tx1_hash)
    hashes.append(iroha.Hash(iroha.Blob(alice_tx2_hash_blob)))
    tx = iroha.ModelQueryBuilder() \
        .createdTime(commons.now()) \
        .queryCounter(1) \
        .creatorAccountId(alice['id']) \
        .getTransactions(hashes) \
        .build()
    return iroha.ModelProtoQuery(tx) \
        .signAndAddSignature(alice['key']).finish()
