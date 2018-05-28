/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PERMISSIONS_HPP
#define IROHA_SHARED_MODEL_PERMISSIONS_HPP

namespace shared_model {
  namespace interface {
    namespace permissions {
      enum class Role {
        kAppendRole,
        kCreateRole,
        kDetachRole,
        kAddAssetQty,
        kSubtractAssetQty,
        kAddPeer,
        kAddSignatory,
        kRemoveSignatory,
        kSetQuorum,
        kCreateAccount,
        kSetDetail,
        kCreateAsset,
        kTransfer,
        kReceive,
        kCreateDomain,
        kReadAssets,
        kGetRoles,
        kGetMyAccount,
        kGetAllAccounts,
        kGetDomainAccounts,
        kGetMySignatories,
        kGetAllSignatories,
        kGetDomainSignatories,
        kGetMyAccAst,
        kGetAllAccAst,
        kGetDomainAccAst,
        kGetMyAccDetail,
        kGetAllAccDetail,
        kGetDomainAccDetail,
        kGetMyAccTxs,
        kGetAllAccTxs,
        kGetDomainAccTxs,
        kGetMyAccAstTxs,
        kGetAllAccAstTxs,
        kGetDomainAccAstTxs,
        kGetMyTxs,
        kGetAllTxs,
        kSetMyQuorum,
        kAddMySignatory,
        kRemoveMySignatory,
        kTransferMyAssets,
        kSetMyAccountDetail,
        kGetBlocks,

        COUNT
      };

      enum class Grantable {
        kAddMySignatory,
        kRemoveMySignatory,
        kSetMyQuorum,
        kSetMyAccountDetail,
        kTransferMyAssets,

        COUNT
      };
    }  // namespace permissions
  }    // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_TRANSACTION_HPP
