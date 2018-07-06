#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
from time import time


def now():
    return int(time() * 1000)


def all_permissions():
    return iroha.RolePermissionSet([
        iroha.Role_kAppendRole,
        iroha.Role_kCreateRole,
        iroha.Role_kDetachRole,
        iroha.Role_kAddAssetQty,
        iroha.Role_kSubtractAssetQty,
        iroha.Role_kAddPeer,
        iroha.Role_kAddSignatory,
        iroha.Role_kRemoveSignatory,
        iroha.Role_kSetQuorum,
        iroha.Role_kCreateAccount,
        iroha.Role_kSetDetail,
        iroha.Role_kCreateAsset,
        iroha.Role_kTransfer,
        iroha.Role_kReceive,
        iroha.Role_kCreateDomain,
        iroha.Role_kReadAssets,
        iroha.Role_kGetRoles,
        iroha.Role_kGetMyAccount,
        iroha.Role_kGetAllAccounts,
        iroha.Role_kGetDomainAccounts,
        iroha.Role_kGetMySignatories,
        iroha.Role_kGetAllSignatories,
        iroha.Role_kGetDomainSignatories,
        iroha.Role_kGetMyAccAst,
        iroha.Role_kGetAllAccAst,
        iroha.Role_kGetDomainAccAst,
        iroha.Role_kGetMyAccDetail,
        iroha.Role_kGetAllAccDetail,
        iroha.Role_kGetDomainAccDetail,
        iroha.Role_kGetMyAccTxs,
        iroha.Role_kGetAllAccTxs,
        iroha.Role_kGetDomainAccTxs,
        iroha.Role_kGetMyAccAstTxs,
        iroha.Role_kGetAllAccAstTxs,
        iroha.Role_kGetDomainAccAstTxs,
        iroha.Role_kGetMyTxs,
        iroha.Role_kGetAllTxs,
        iroha.Role_kSetMyQuorum,
        iroha.Role_kAddMySignatory,
        iroha.Role_kRemoveMySignatory,
        iroha.Role_kTransferMyAssets,
        iroha.Role_kSetMyAccountDetail,
        iroha.Role_kGetBlocks
    ])


def new_user(user_id):
    key = iroha.ModelCrypto().generateKeypair()
    if user_id.lower().startswith('admin'):
        print('K{}'.format(key.privateKey().hex()))
    return {
        'id': user_id,
        'key': key
    }


def hex(generator):
    """
    Decorator for transactions' and queries generators.

    Allows preserving the type of binaries for Binary Testing Framework.
    """
    prefix = 'T' if generator.__name__.lower().endswith('tx') else 'Q'
    print('{}{}'.format(prefix, generator().hex()))
