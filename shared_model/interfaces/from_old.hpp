/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <map>
#include <set>
#include <string>
#include "interfaces/permissions.hpp"

namespace shared_model {
  namespace interface {
    namespace permissions {
      static inline Role fromOldR(const std::string &s) {
        static std::map<std::string, Role> map = {
            {"can_append_role", Role::kAppendRole},
            {"can_create_role", Role::kCreateRole},
            {"can_detach_role", Role::kDetachRole},
            {"can_add_asset_qty", Role::kAddAssetQty},
            {"can_subtract_asset_qty", Role::kSubtractAssetQty},
            {"can_add_peer", Role::kAddPeer},
            {"can_add_signatory", Role::kAddSignatory},
            {"can_remove_signatory", Role::kRemoveSignatory},
            {"can_set_quorum", Role::kSetQuorum},
            {"can_create_account", Role::kCreateAccount},
            {"can_set_detail", Role::kSetDetail},
            {"can_create_asset", Role::kCreateAsset},
            {"can_transfer", Role::kTransfer},
            {"can_receive", Role::kReceive},
            {"can_create_domain", Role::kCreateDomain},
            {"can_read_assets", Role::kReadAssets},
            {"can_get_roles", Role::kGetRoles},
            {"can_get_my_account", Role::kGetMyAccount},
            {"can_get_all_accounts", Role::kGetAllAccounts},
            {"can_get_domain_accounts", Role::kGetDomainAccounts},
            {"can_get_my_signatories", Role::kGetMySignatories},
            {"can_get_all_signatories", Role::kGetAllSignatories},
            {"can_get_domain_signatories", Role::kGetDomainSignatories},
            {"can_get_my_acc_ast", Role::kGetMyAccAst},
            {"can_get_all_acc_ast", Role::kGetAllAccAst},
            {"can_get_domain_acc_ast", Role::kGetDomainAccAst},
            {"can_get_my_acc_detail", Role::kGetMyAccDetail},
            {"can_get_all_acc_detail", Role::kGetAllAccDetail},
            {"can_get_domain_acc_detail", Role::kGetDomainAccDetail},
            {"can_get_my_acc_txs", Role::kGetMyAccTxs},
            {"can_get_all_acc_txs", Role::kGetAllAccTxs},
            {"can_get_domain_acc_txs", Role::kGetDomainAccTxs},
            {"can_get_my_acc_ast_txs", Role::kGetMyAccAstTxs},
            {"can_get_all_acc_ast_txs", Role::kGetAllAccAstTxs},
            {"can_get_domain_acc_ast_txs", Role::kGetDomainAccAstTxs},
            {"can_get_my_txs", Role::kGetMyTxs},
            {"can_get_all_txs", Role::kGetAllTxs},
            {"can_get_blocks", Role::kGetBlocks},
            {"can_grant_can_set_my_quorum", Role::kSetMyQuorum},
            {"can_grant_can_add_my_signatory", Role::kAddMySignatory},
            {"can_grant_can_remove_my_signatory", Role::kRemoveMySignatory},
            {"can_grant_can_transfer_my_assets", Role::kTransferMyAssets},
            {"can_grant_can_set_my_account_detail", Role::kSetMyAccountDetail}};
        std::cout << (map.find(s) == map.end() ? "cannot find" + s + "\n" : "");
        return map.at(s);
      }

      static inline RolePermissionSet fromOldR(const std::set<std::string> &s) {
        RolePermissionSet set;
        for (auto &el : s) {
          set.set(fromOldR(el));
        }
        return set;
      }

      static inline Grantable fromOldG(const std::string &s) {
        static std::map<std::string, Grantable> map = {
            {"can_add_my_signatory", Grantable::kAddMySignatory},
            {"can_remove_my_signatory", Grantable::kRemoveMySignatory},
            {"can_set_my_quorum", Grantable::kSetMyQuorum},
            {"can_set_my_account_detail", Grantable::kSetMyAccountDetail},
            {"can_transfer_my_assets", Grantable::kTransferMyAssets}};
        return map.at(s);
      }

      static inline GrantablePermissionSet fromOldG(
          const std::set<std::string> &s) {
        GrantablePermissionSet set;
        for (auto &el : s) {
          set.set(fromOldG(el));
        }
        return set;
      }

    }  // namespace permissions
  }    // namespace interface
}  // namespace shared_model
