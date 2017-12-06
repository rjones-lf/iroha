/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_PERMISSIONS_HPP
#define IROHA_PERMISSIONS_HPP

#include <string>
#include <set>

namespace iroha {
  namespace model {

    const std::string can_append_role = "CanAppendRole";
    const std::string can_create_role = "CanCreateRole";
    const std::string can_add_asset_qty = "CanAddAssetQuantity";
    const std::string can_subtract_asset_qty = "CanSubtractAssetQuantity";
    const std::string can_add_peer = "CanAddPeer";
    const std::string can_add_signatory = "CanAddSignatory";
    const std::string can_create_account = "CanCreateAccount";
    const std::string can_create_asset = "CanCreateAsset";
    const std::string can_create_domain = "CanCreateDomain";
    const std::string can_remove_signatory = "CanRemoveSignatory";
    const std::string can_set_quorum = "CanSetQuorum";
    const std::string can_transfer = "CanTransfer";
    const std::string can_receive = "CanReceive";

    // ---------|Query permissions|-------------
    const std::string can_read_assets = "CanReadAssets";
    const std::string can_get_roles = "CanGetRoles";
    const std::string can_get_my_account = "CanGetMyAccount";
    const std::string can_get_all_accounts = "CanGetAllAccounts";

    const std::string can_get_my_signatories = "CanGetMySignatories";
    const std::string can_get_all_signatories = "CanGetAllSignatories";

    const std::string can_get_my_acc_ast = "CanGetMyAccountAssets";
    const std::string can_get_all_acc_ast = "CanGetAllAccountAssets";

    const std::string can_get_my_acc_txs = "CanGetMyAccountTransactions";
    const std::string can_get_all_acc_txs = "CanGetAllAccountTransactions";

    const std::string can_get_my_acc_ast_txs =
        "CanGetMyAccountAssetsTransactions";
    const std::string can_get_all_acc_ast_txs =
        "CanGetAllAccountAssetsTransactions";

    const std::set<std::string> read_self_group = {
        can_get_my_account,
        can_get_my_acc_txs,
        can_get_my_acc_ast,
        can_get_my_acc_ast_txs,
        can_get_my_signatories};

    const std::set<std::string> read_all_group = {
        can_get_all_accounts,
        can_get_all_acc_txs,
        can_get_all_acc_ast,
        can_get_all_acc_ast_txs,
        can_get_all_signatories,
        can_get_roles,
        can_read_assets};
    const std::string can_grant = "CanGrant";
    const std::set<std::string> grant_group = {
        can_grant + can_set_quorum,
        can_grant + can_add_signatory,
        can_grant + can_remove_signatory,
        can_grant + can_transfer
        };

    const std::set<std::string> edit_self_group = {
        can_set_quorum, can_add_signatory, can_remove_signatory};

    const std::set<std::string> asset_creator_group = {
        can_create_asset, can_add_asset_qty};

    const std::set<std::string> all_perm_group = {
        can_get_my_account,
        can_get_my_acc_txs,
        can_get_my_acc_ast,
        can_get_my_acc_ast_txs,
        can_get_my_signatories,
        can_get_all_accounts,
        can_get_all_acc_txs,
        can_get_all_acc_ast,
        can_get_all_acc_ast_txs,
        can_get_all_signatories,
        can_get_roles,
        can_read_assets,
        can_grant + can_set_quorum,
        can_grant + can_add_signatory,
        can_grant + can_remove_signatory,
        can_grant + can_transfer,
        can_set_quorum,
        can_add_signatory,
        can_remove_signatory,
        can_create_asset,
        can_add_asset_qty,
        can_subtract_asset_qty,
        can_append_role,
        can_create_role,
        can_create_account,
        can_add_peer,
        can_create_domain};

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_PERMISSIONS_HPP
