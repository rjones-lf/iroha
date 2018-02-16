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

#include <set>
#include <string>

namespace iroha {
  namespace model {

    const std::string can_append_role = "can_append_role";
    const std::string can_create_role = "can_create_role";
    const std::string can_detach_role = "can_detach_role";
    const std::string can_add_asset_qty = "can_add_asset_qty";
    const std::string can_subtract_asset_qty = "can_subtract_asset_qty";
    const std::string can_add_peer = "can_add_peer";
    const std::string can_add_signatory = "can_add_signatory";
    const std::string can_create_account = "can_create_account";
    const std::string can_create_asset = "can_create_asset";
    const std::string can_create_domain = "can_create_domain";
    const std::string can_remove_signatory = "can_remove_signatory";
    const std::string can_set_quorum = "can_set_quorum";
    const std::string can_transfer = "can_transfer";
    const std::string can_receive = "can_receive";
    const std::string can_set_detail = "can_set_detail";

    // ---------|Query permissions|-------------
    const std::string can_read_assets = "can_read_assets";
    const std::string can_get_roles = "can_get_roles";

    const std::string can_get_my_account = "can_get_my_account";
    const std::string can_get_all_accounts = "can_get_all_accounts";
    const std::string can_get_domain_accounts = "can_get_domain_accounts";

    const std::string can_get_my_signatories = "can_get_my_signatories";
    const std::string can_get_all_signatories = "can_get_all_signatories";
    const std::string can_get_domain_signatories = "can_get_domain_signatories";

    const std::string can_get_my_acc_ast = "can_get_my_acc_ast";
    const std::string can_get_all_acc_ast = "can_get_all_acc_ast";
    const std::string can_get_domain_acc_ast = "can_get_domain_acc_ast";
    const std::string can_get_my_acc_detail = "can_get_my_acc_detail";
    const std::string can_get_all_acc_detail = "can_get_all_acc_detail";
    const std::string can_get_domain_acc_detail = "can_get_domain_acc_detail";

    const std::string can_get_my_acc_txs = "can_get_my_acc_txs";
    const std::string can_get_all_acc_txs = "can_get_all_acc_txs";
    const std::string can_get_domain_acc_txs = "can_get_domain_acc_txs";

    const std::string can_get_my_acc_ast_txs = "can_get_my_acc_ast_txs";
    const std::string can_get_all_acc_ast_txs = "can_get_all_acc_ast_txs";
    const std::string can_get_domain_acc_ast_txs = "can_get_domain_acc_ast_txs";

    const std::string can_get_my_txs = "can_get_my_txs";
    const std::string can_get_all_txs = "can_get_all_txs";

    const std::set<std::string> read_self_group = {can_get_my_account,
                                                   can_get_my_acc_ast,
                                                   can_get_my_acc_txs,
                                                   can_get_my_acc_ast_txs,
                                                   can_get_my_txs,
                                                   can_get_my_signatories};

    const std::set<std::string> read_all_group = {can_get_all_accounts,
                                                  can_get_all_acc_ast,
                                                  can_get_all_acc_txs,
                                                  can_get_all_acc_ast_txs,
                                                  can_get_all_txs,
                                                  can_get_all_signatories,
                                                  can_get_roles,
                                                  can_read_assets};

    const std::set<std::string> read_domain_group = {can_get_domain_accounts,
                                                     can_get_domain_acc_txs,
                                                     can_get_domain_acc_ast,
                                                     can_get_domain_acc_detail,
                                                     can_get_domain_acc_ast_txs,
                                                     can_get_domain_signatories,
                                                     can_get_my_acc_detail};

    const std::string can_grant = "can_grant_";
    const std::set<std::string> grant_group = {can_grant + can_set_quorum,
                                               can_grant + can_add_signatory,
                                               can_grant + can_remove_signatory,
                                               can_grant + can_transfer,
                                               can_grant + can_set_detail};

    const std::set<std::string> edit_self_group = {
        can_set_quorum, can_add_signatory, can_remove_signatory};

    const std::set<std::string> asset_creator_group = {can_create_asset,
                                                       can_add_asset_qty};

    const std::set<std::string> all_perm_group = {
        can_get_my_account,
        can_get_my_acc_ast,
        can_get_my_acc_detail,
        can_get_my_acc_txs,
        can_get_my_acc_ast_txs,
        can_get_my_txs,
        can_get_my_signatories,
        can_get_all_accounts,
        can_get_all_acc_ast,
        can_get_all_acc_txs,
        can_get_all_acc_ast_txs,
        can_get_all_txs,
        can_get_all_signatories,
        can_get_roles,
        can_read_assets,
        can_grant + can_set_quorum,
        can_grant + can_add_signatory,
        can_grant + can_remove_signatory,
        can_grant + can_transfer,
        can_grant + can_set_detail,
        can_set_quorum,
        can_add_signatory,
        can_remove_signatory,
        can_create_asset,
        can_add_asset_qty,
        can_subtract_asset_qty,
        can_append_role,
        can_create_role,
        can_detach_role,
        can_create_account,
        can_add_peer,
        can_create_domain};

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_PERMISSIONS_HPP
