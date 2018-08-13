#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
import commons

admin = commons.new_user('admin@first')
alice = commons.new_user('alice@second')

admin_tx1_hash = None
admin_tx2_hash_blob = None


@commons.hex
def genesis_tx():
    test_permissions = iroha.RolePermissionSet([iroha.Role_kGetAllTxs])
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
def admin_action_1_tx():
    global admin_tx1_hash
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(admin['id']) \
        .createAsset('coin', 'second', 2) \
        .build()
    admin_tx1_hash = tx.hash()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(admin['key']).finish()


@commons.hex
def admin_action_2_tx():
    global admin_tx2_hash_blob
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(admin['id']) \
        .setAccountDetail(admin['id'], 'hyperledger', 'iroha') \
        .build()
    admin_tx2_hash_blob = tx.hash().blob()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(admin['key']).finish()


@commons.hex
def transactions_query():
    hashes = iroha.HashVector()
    hashes.append(admin_tx1_hash)
    hashes.append(iroha.Hash(iroha.Blob(admin_tx2_hash_blob)))
    tx = iroha.ModelQueryBuilder() \
        .createdTime(commons.now()) \
        .queryCounter(1) \
        .creatorAccountId(alice['id']) \
        .getTransactions(hashes) \
        .build()
    return iroha.ModelProtoQuery(tx) \
        .signAndAddSignature(alice['key']).finish()
