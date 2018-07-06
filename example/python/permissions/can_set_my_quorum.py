#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
import commons

admin = commons.new_user('admin@test')
alice = commons.new_user('alice@test')
bob = commons.new_user('bob@test')


@commons.hex
def genesis_tx():
    test_permissions = iroha.RolePermissionSet([
        iroha.Role_kSetMyQuorum,
        iroha.Role_kAddSignatory
    ])
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(admin['id']) \
        .addPeer('0.0.0.0:50541', admin['key'].publicKey()) \
        .createRole('admin_role', commons.all_permissions()) \
        .createRole('test_role', test_permissions) \
        .createDomain('test', 'test_role') \
        .createAccount('admin', 'test', admin['key'].publicKey()) \
        .createAccount('alice', 'test', alice['key'].publicKey()) \
        .createAccount('bob', 'test', bob['key'].publicKey()) \
        .appendRole(admin['id'], 'admin_role') \
        .build()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(admin['key']).finish()


@commons.hex
def grant_can_set_my_quorum_tx():
    extra_key = iroha.ModelCrypto().generateKeypair()
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(alice['id']) \
        .grantPermission(bob['id'], iroha.Grantable_kSetMyQuorum) \
        .addSignatory(alice['id'], extra_key.publicKey()) \
        .build()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(alice['key']).finish()


@commons.hex
def set_quorum_tx():
    tx = iroha.ModelTransactionBuilder() \
        .createdTime(commons.now()) \
        .creatorAccountId(bob['id']) \
        .setAccountQuorum(alice['id'], 2) \
        .build()
    return iroha.ModelProtoTransaction(tx) \
        .signAndAddSignature(bob['key']).finish()
