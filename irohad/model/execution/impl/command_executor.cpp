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

#include "model/execution/command_executor.hpp"
#include <algorithm>
#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/append_role.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/create_role.hpp"
#include "model/commands/detach_role.hpp"
#include "model/commands/grant_permission.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/revoke_permission.hpp"
#include "model/commands/set_account_detail.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/subtract_asset_quantity.hpp"
#include "model/commands/transfer_asset.hpp"
#include "model/execution/common_executor.hpp"
#include "model/permissions.hpp"
#include "validator/domain_name_validator.hpp"

using namespace iroha::ametsuchi;

namespace iroha {
  namespace model {

    // ----------------------------| Common |-----------------------------

    CommandExecutor::CommandExecutor() {
      log_ = logger::log("DefaultCommandExecutorLogger");
    }

    bool CommandExecutor::validate(const Command &command,
                                   WsvQuery &queries,
                                   const std::string &creator_account_id) {
      return hasPermissions(command, queries, creator_account_id)
          and isValid(command, queries, creator_account_id);
    }

    // ----------------------------| Append Role |-----------------------------
    AppendRoleExecutor::AppendRoleExecutor() {
      log_ = logger::log("AppendRoleExecutor");
    }

    bool AppendRoleExecutor::execute(const Command &command,
                                     ametsuchi::WsvQuery &queries,
                                     ametsuchi::WsvCommand &commands,
                                     const std::string &creator_account_id) {
      auto cmd_value = static_cast<const AppendRole &>(command);

      return commands.insertAccountRole(cmd_value.account_id,
                                        cmd_value.role_name);
    }

    bool AppendRoleExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      return checkAccountRolePermission(
          creator_account_id, queries, can_append_role);
    }

    bool AppendRoleExecutor::isValid(const Command &command,
                                     ametsuchi::WsvQuery &queries,
                                     const std::string &creator_account_id) {

      auto cmd_value = static_cast<const AppendRole &>(command);
      auto role_permissions = queries.getRolePermissions(cmd_value.role_name);
      auto account_roles = queries.getAccountRoles(creator_account_id);

      if (not role_permissions.has_value() or not account_roles.has_value()) {
        return false;
      }

      std::set<std::string> account_permissions;
      for (const auto &role : *account_roles) {
        auto permissions = queries.getRolePermissions(role);
        if (not permissions.has_value())
          continue;
        for (const auto &permission : *permissions)
          account_permissions.insert(permission);
      }

      return std::none_of((*role_permissions).begin(),
                          (*role_permissions).end(),
                          [&account_permissions](const auto &perm) {
                            return account_permissions.find(perm) == account_permissions.end();
                          });
    }

    // ----------------------------| Detach Role |-----------------------------
    DetachRoleExecutor::DetachRoleExecutor() {
      log_ = logger::log("DetachRoleExecutor");
    }

    bool DetachRoleExecutor::execute(const Command &command,
                                     ametsuchi::WsvQuery &queries,
                                     ametsuchi::WsvCommand &commands,
                                     const std::string &creator_account_id) {
      auto cmd_value = static_cast<const DetachRole &>(command);

      return commands.deleteAccountRole(cmd_value.account_id,
                                        cmd_value.role_name);
    }

    bool DetachRoleExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      return checkAccountRolePermission(
          creator_account_id, queries, can_detach_role);
    }

    bool DetachRoleExecutor::isValid(const Command &command,
                                     ametsuchi::WsvQuery &queries,
                                     const std::string &creator_account_id) {
      return true;
    }

    // ----------------------------| Create Role |-----------------------------

    CreateRoleExecutor::CreateRoleExecutor() {
      log_ = logger::log("CreateRoleExecutor");
    }

    bool CreateRoleExecutor::execute(const Command &command,
                                     ametsuchi::WsvQuery &queries,
                                     ametsuchi::WsvCommand &commands,
                                     const std::string &creator_account_id) {
      auto cmd_value = static_cast<const CreateRole &>(command);

      return commands.insertRole(cmd_value.role_name)
          and commands.insertRolePermissions(cmd_value.role_name,
                                             cmd_value.permissions);
    }

    bool CreateRoleExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      return checkAccountRolePermission(
          creator_account_id, queries, can_create_role);
    }

    bool CreateRoleExecutor::isValid(const Command &command,
                                     ametsuchi::WsvQuery &queries,
                                     const std::string &creator_account_id) {
      auto cmd_value = static_cast<const CreateRole &>(command);
      cmd_value.role_name.size();

      auto role_is_a_subset =
          std::all_of(cmd_value.permissions.begin(),
                      cmd_value.permissions.end(),
                      [&queries, &creator_account_id](auto perm) {
                        return checkAccountRolePermission(
                            creator_account_id, queries, perm);
                      });

      return role_is_a_subset and not cmd_value.role_name.empty()
          and cmd_value.role_name.size() < 8 and
          // Role must be well-formed (no system symbols)
          std::all_of(std::begin(cmd_value.role_name),
                      std::end(cmd_value.role_name),
                      [](char c) { return std::isalnum(c) and islower(c); });
    }

    // --------------------|Grant Permission|-----------------------
    GrantPermissionExecutor::GrantPermissionExecutor() {
      log_ = logger::log("GrantPermissionExecutor");
    }

    bool GrantPermissionExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      if (creator_account_id.empty()) {
        log_->info("Creator account is empty");
        return false;
      }
      auto cmd_value = static_cast<const GrantPermission &>(command);
      return commands.insertAccountGrantablePermission(
          cmd_value.account_id, creator_account_id, cmd_value.permission_name);
    }

    bool GrantPermissionExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const GrantPermission &>(command);
      return checkAccountRolePermission(
          creator_account_id, queries, can_grant + cmd_value.permission_name);
    }

    bool GrantPermissionExecutor::isValid(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      // TODO: no additional checks ?
      return true;
    }

    // --------------------------|Revoke
    // Permission|-----------------------------
    RevokePermissionExecutor::RevokePermissionExecutor() {
      log_ = logger::log("RevokePermissionExecutor");
    }

    bool RevokePermissionExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      if (creator_account_id.empty()) {
        log_->info("Creator account is empty");
        return false;
      }
      auto cmd_value = static_cast<const RevokePermission &>(command);
      return commands.deleteAccountGrantablePermission(
          cmd_value.account_id, creator_account_id, cmd_value.permission_name);
    }

    bool RevokePermissionExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const GrantPermission &>(command);
      // Target account must have permission on creator's account -> creator can
      // revoke his permission
      return queries.hasAccountGrantablePermission(
          cmd_value.account_id, creator_account_id, cmd_value.permission_name);
    }

    bool RevokePermissionExecutor::isValid(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      // TODO: no checks needed ?
      return true;
    }

    // ----------------------------|AddAssetQuantity|-----------------------------

    AddAssetQuantityExecutor::AddAssetQuantityExecutor() {
      log_ = logger::log("AddAssetQuantityExecutor");
    }

    bool AddAssetQuantityExecutor::execute(
        const Command &command,
        WsvQuery &queries,
        WsvCommand &commands,
        const std::string &creator_account_id) {
      auto add_asset_quantity = static_cast<const AddAssetQuantity &>(command);

      auto asset = queries.getAsset(add_asset_quantity.asset_id);
      if (not asset.has_value()) {
        log_->info("asset {} is absent", add_asset_quantity.asset_id);
        return false;
      }
      auto precision = asset.value().precision;

      if (add_asset_quantity.amount.getPrecision() != precision) {
        log_->info("amount is wrongly formed:");
        return false;
      }
      if (not queries.getAccount(add_asset_quantity.account_id).has_value()) {
        log_->info("amount {} is absent", add_asset_quantity.account_id);
        return false;
      }
      auto account_asset = queries.getAccountAsset(
          add_asset_quantity.account_id, add_asset_quantity.asset_id);
      if (not account_asset.has_value()) {
        log_->info("create wallet {} for {}",
                   add_asset_quantity.asset_id,
                   add_asset_quantity.account_id);

        account_asset = AccountAsset();
        account_asset->asset_id = add_asset_quantity.asset_id;
        account_asset->account_id = add_asset_quantity.account_id;
        account_asset->balance = add_asset_quantity.amount;
      } else {
        auto account_asset_value = account_asset.value();

        auto new_balance =
            account_asset_value.balance + add_asset_quantity.amount;
        if (not new_balance.has_value()) {
          return false;
        }
        account_asset->balance = new_balance.value();
      }

      // accountAsset.value().balance += amount;
      return commands.upsertAccountAsset(account_asset.value());
    }

    bool AddAssetQuantityExecutor::hasPermissions(
        const Command &command,
        WsvQuery &queries,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const AddAssetQuantity &>(command);
      // Check if creator has MoneyCreator permission.
      // One can only add to his/her account
      // TODO: In future: Separate money creation for distinct assets
      return creator_account_id == cmd_value.account_id
          and checkAccountRolePermission(
                  creator_account_id, queries, can_add_asset_qty);
    }

    bool AddAssetQuantityExecutor::isValid(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto add_asset_quantity = static_cast<const AddAssetQuantity &>(command);
      return true;
    }

    // ----------------------------|SubtractAssetQuantity|-----------------------------

    SubtractAssetQuantityExecutor::SubtractAssetQuantityExecutor() {
      log_ = logger::log("SubtractAssetQuantityExecutor");
    }

      bool SubtractAssetQuantityExecutor::execute(const Command &command,
                                             WsvQuery &queries,
                                             WsvCommand &commands,
        const std::string &creator_account_id) {
        auto subtract_asset_quantity = static_cast<const SubtractAssetQuantity &>(command);

      auto asset = queries.getAsset(subtract_asset_quantity.asset_id);
      if (not asset) {
        log_->info("asset {} is absent", subtract_asset_quantity.asset_id);
        return false;
      }
      auto precision = asset.value().precision;

      if (subtract_asset_quantity.amount.getPrecision() != precision) {
        log_->info("amount is wrongly formed");
        return false;
      }
      auto account_asset = queries.getAccountAsset(
          subtract_asset_quantity.account_id, subtract_asset_quantity.asset_id);
      if (not account_asset.has_value()) {
        log_->info("{} do not have {}",
                   subtract_asset_quantity.account_id,
                   subtract_asset_quantity.asset_id);
        return false;
      }
      auto account_asset_value = account_asset.value();

      auto new_balance =
          account_asset_value.balance - subtract_asset_quantity.amount;
      if (not new_balance.has_value()) {
        log_->info("Not sufficient amount");
        return false;
      }
      account_asset->balance = new_balance.value();

      // accountAsset.value().balance -= amount;
      return commands.upsertAccountAsset(account_asset.value());
    }

      bool SubtractAssetQuantityExecutor::hasPermissions(const Command &command,
                                                    WsvQuery &queries,
                                                    const std::string &creator_account_id) {
        auto cmd_value = static_cast<const SubtractAssetQuantity &>(command);
        return creator_account_id == cmd_value.account_id
               and checkAccountRolePermission(
          creator_account_id, queries, can_subtract_asset_qty);
      }

      bool SubtractAssetQuantityExecutor::isValid(const Command &command,
                                             ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
        return true;
      }

    // --------------------------|AddPeer|------------------------------

    AddPeerExecutor::AddPeerExecutor() {
      log_ = logger::log("AddPeerExecutor");
    }

    bool AddPeerExecutor::execute(const Command &command,
                                  ametsuchi::WsvQuery &queries,
                                  ametsuchi::WsvCommand &commands,
                                  const std::string &creator_account_id) {
      auto add_peer = static_cast<const AddPeer &>(command);

      Peer peer;
      peer.address = add_peer.address;
      peer.pubkey = add_peer.peer_key;
      // Will return false if peer is not unique
      return commands.insertPeer(peer);
    }

    bool AddPeerExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      return checkAccountRolePermission(
          creator_account_id, queries, can_add_peer);
    }

    bool AddPeerExecutor::isValid(const Command &command,
                                  ametsuchi::WsvQuery &queries,
                                  const std::string &creator_account_id) {
      // TODO: check that address is formed right
      return true;
    }

    // -------------------------|AddSignatory|--------------------------
    AddSignatoryExecutor::AddSignatoryExecutor() {
      log_ = logger::log("AddSignatoryExecutor");
    }

    bool AddSignatoryExecutor::execute(const Command &command,
                                       ametsuchi::WsvQuery &queries,
                                       ametsuchi::WsvCommand &commands,
                                       const std::string &creator_account_id) {
      auto add_signatory = static_cast<const AddSignatory &>(command);

      return commands.insertSignatory(add_signatory.pubkey)
          && commands.insertAccountSignatory(add_signatory.account_id,
                                             add_signatory.pubkey);
    }

    bool AddSignatoryExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto add_signatory = static_cast<const AddSignatory &>(command);

      return
          // Case 1. When command creator wants to add signatory to their
          // account and he has permission CanAddSignatory
          (add_signatory.account_id == creator_account_id
           and checkAccountRolePermission(
                   creator_account_id, queries, can_add_signatory))
          or
          // Case 2. Creator has granted permission for it
          (queries.hasAccountGrantablePermission(
              creator_account_id, add_signatory.account_id, can_add_signatory));
    }

    bool AddSignatoryExecutor::isValid(const Command &command,
                                       ametsuchi::WsvQuery &queries,
                                       const std::string &creator_account_id) {
      return true;
    }

    // ------------------------------|CreateAccount|------------------------------
    CreateAccountExecutor::CreateAccountExecutor() {
      log_ = logger::log("CreateAccountExecutor");
    }

    bool CreateAccountExecutor::execute(const Command &command,
                                        ametsuchi::WsvQuery &queries,
                                        ametsuchi::WsvCommand &commands,
                                        const std::string &creator_account_id) {
      auto create_account = static_cast<const CreateAccount &>(command);

      Account account;
      account.account_id =
          create_account.account_name + "@" + create_account.domain_id;

      account.domain_id = create_account.domain_id;
      account.quorum = 1;
      account.json_data = "{}";
      auto domain = queries.getDomain(create_account.domain_id);
      if (not domain.has_value()) {
        log_->info("Domain {} not found", create_account.domain_id);
        return false;
      }
      // TODO: remove insert signatory from here ?
      return commands.insertSignatory(create_account.pubkey)
          and commands.insertAccount(account)
          and commands.insertAccountSignatory(account.account_id,
                                              create_account.pubkey)
          and commands.insertAccountRole(account.account_id,
                                         domain.value().default_role);
    }

    bool CreateAccountExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      // Creator must have permission to create account
      return checkAccountRolePermission(
          creator_account_id, queries, can_create_account);
    }

    bool CreateAccountExecutor::isValid(const Command &command,
                                        ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
      auto create_account = static_cast<const CreateAccount &>(command);

      return
          // Name is within some range
          not create_account.account_name.empty()
          // Account must be well-formed (no system symbols)
          and validator::isValidDomainName(create_account.account_name);
    }

    // --------------------------|CreateAsset|---------------------------

    CreateAssetExecutor::CreateAssetExecutor() {
      log_ = logger::log("CreateAssetExecutor");
    }

    bool CreateAssetExecutor::execute(const Command &command,
                                      ametsuchi::WsvQuery &queries,
                                      ametsuchi::WsvCommand &commands,
                                      const std::string &creator_account_id) {
      auto create_asset = static_cast<const CreateAsset &>(command);

      Asset new_asset;
      new_asset.asset_id =
          create_asset.asset_name + "#" + create_asset.domain_id;
      new_asset.domain_id = create_asset.domain_id;
      new_asset.precision = create_asset.precision;
      // The insert will fail if asset already exist
      return commands.insertAsset(new_asset);
    }

    bool CreateAssetExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      // Creator must have permission to create assets
      return checkAccountRolePermission(
          creator_account_id, queries, can_create_asset);
    }

    bool CreateAssetExecutor::isValid(const Command &command,
                                      ametsuchi::WsvQuery &queries,
                                      const std::string &creator_account_id) {
      auto create_asset = static_cast<const CreateAsset &>(command);

      return
          // Name is within some range
          not create_asset.asset_name.empty()
          && create_asset.asset_name.size() < 10 &&
          // Account must be well-formed (no system symbols)
          std::all_of(std::begin(create_asset.asset_name),
                      std::end(create_asset.asset_name),
                      [](char c) { return std::isalnum(c); });
    }

    // ------------------------|CreateDomain|---------------------------

    CreateDomainExecutor::CreateDomainExecutor() {
      log_ = logger::log("CreateDomainExecutor");
    }

    bool CreateDomainExecutor::execute(const Command &command,
                                       ametsuchi::WsvQuery &queries,
                                       ametsuchi::WsvCommand &commands,
                                       const std::string &creator_account_id) {
      auto create_domain = static_cast<const CreateDomain &>(command);

      Domain new_domain;
      new_domain.domain_id = create_domain.domain_id;
      new_domain.default_role = create_domain.user_default_role;
      // The insert will fail if domain already exist
      return commands.insertDomain(new_domain);
    }

    bool CreateDomainExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      // Creator must have permission to create domains
      return checkAccountRolePermission(
          creator_account_id, queries, can_create_domain);
    }

    bool CreateDomainExecutor::isValid(const Command &command,
                                       ametsuchi::WsvQuery &queries,
                                       const std::string &creator_account_id) {
      auto create_domain = static_cast<const CreateDomain &>(command);

      return
          // Name is within some range
          not create_domain.domain_id.empty()
          and create_domain.domain_id.size() < 10 and
          // Account must be well-formed (no system symbols)
          std::all_of(std::begin(create_domain.domain_id),
                      std::end(create_domain.domain_id),
                      [](char c) { return std::isalnum(c); });
    }

    // --------------------|RemoveSignatory|--------------------

    RemoveSignatoryExecutor::RemoveSignatoryExecutor() {
      log_ = logger::log("RemoveSignatoryExecutor");
    }

    bool RemoveSignatoryExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto remove_signatory = static_cast<const RemoveSignatory &>(command);

      // Delete will fail if account signatory doesn't exist
      return commands.deleteAccountSignatory(remove_signatory.account_id,
                                             remove_signatory.pubkey)
          && commands.deleteSignatory(remove_signatory.pubkey);
    }

    bool RemoveSignatoryExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto remove_signatory = static_cast<const RemoveSignatory &>(command);

      // Two cases possible.

      return
          // 1. Creator removes signatory from their account, and he must have
          // permission on it
          (creator_account_id == remove_signatory.account_id
           and checkAccountRolePermission(
                   creator_account_id, queries, can_remove_signatory))
          // 2. Creator has granted permission on removal
          or (queries.hasAccountGrantablePermission(creator_account_id,
                                                    remove_signatory.account_id,
                                                    can_remove_signatory));
    }

    bool RemoveSignatoryExecutor::isValid(const Command &command,
                                          ametsuchi::WsvQuery &queries, const std::string &creator_account_id) {
      auto remove_signatory = static_cast<const RemoveSignatory &>(command);

      auto account = queries.getAccount(remove_signatory.account_id);
      auto signatories = queries.getSignatories(remove_signatory.account_id);

      if (not(account.has_value() and signatories.has_value())) {
        // No account or signatories found
        return false;
      }

      auto newSignatoriesSize = signatories.value().size() - 1;

      // You can't remove if size of rest signatories less than the quorum
      return newSignatoriesSize >= account.value().quorum;
    }

    // -----------------------|SetAccountDetail|-------------------------

    SetAccountDetailExecutor::SetAccountDetailExecutor() {
      log_ = logger::log("SetAccountDetailExecutor");
    }

    bool SetAccountDetailExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto cmd = static_cast<const SetAccountDetail &>(command);
      auto creator = creator_account_id;
      if (creator_account_id.empty()) {
        // When creator is not known, it is genesis block
        creator = "genesis";
      }
      return commands.setAccountKV(cmd.account_id, creator, cmd.key, cmd.value);
    }

    bool SetAccountDetailExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto cmd = static_cast<const SetAccountDetail &>(command);

      return
          // Case 1. Creator set details for his account
          creator_account_id == cmd.account_id or
          // Case 2. Creator has grantable permission to set account key/value
          queries.hasAccountGrantablePermission(
              creator_account_id, cmd.account_id, can_set_detail);
    }

    bool SetAccountDetailExecutor::isValid(const Command &command,
                                           ametsuchi::WsvQuery &queries, const std::string &creator_account_id) {
      return true;
    }

    // -----------------------|SetQuorum|-------------------------

    SetQuorumExecutor::SetQuorumExecutor() {
      log_ = logger::log("SetQuorumExecutor");
    }

    bool SetQuorumExecutor::execute(const Command &command,
                                    ametsuchi::WsvQuery &queries,
                                    ametsuchi::WsvCommand &commands,
                                    const std::string &creator_account_id) {
      auto set_quorum = static_cast<const SetQuorum &>(command);

      auto account = queries.getAccount(set_quorum.account_id);
      if (not account.has_value()) {
        log_->info("absent account {}", set_quorum.account_id);
        return false;
      }
      account.value().quorum = set_quorum.new_quorum;
      return commands.updateAccount(account.value());
    }

    bool SetQuorumExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto set_quorum = static_cast<const SetQuorum &>(command);

      return
          // 1. Creator set quorum for his account -> must have permission
          (creator_account_id == set_quorum.account_id
           and checkAccountRolePermission(
                   creator_account_id, queries, can_set_quorum))
          // 2. Creator has granted permission on it
          or (queries.hasAccountGrantablePermission(
                 creator_account_id, set_quorum.account_id, can_set_quorum));
    }

    bool SetQuorumExecutor::isValid(const Command &command,
                                    ametsuchi::WsvQuery &queries, const std::string &creator_account_id) {
      auto set_quorum = static_cast<const SetQuorum &>(command);
      auto signatories = queries.getSignatories(set_quorum.account_id);

      if (not(signatories.has_value())) {
        // No  signatories of an account found
        return false;
      }
      // You can't remove if size of rest signatories less than the quorum
      return set_quorum.new_quorum > 0 and set_quorum.new_quorum < 10
          and signatories.value().size() >= set_quorum.new_quorum;
    }

    // ------------------------|TransferAsset|-------------------------
    TransferAssetExecutor::TransferAssetExecutor() {
      log_ = logger::log("TransferAssetExecutor");
    }

    bool TransferAssetExecutor::execute(const Command &command,
                                        ametsuchi::WsvQuery &queries,
                                        ametsuchi::WsvCommand &commands,
                                        const std::string &creator_account_id) {
      auto transfer_asset = static_cast<const TransferAsset &>(command);

      auto src_account_asset = queries.getAccountAsset(
          transfer_asset.src_account_id, transfer_asset.asset_id);
      if (not src_account_asset.has_value()) {
        log_->info("asset {} is absent of {}",
                   transfer_asset.asset_id,
                   transfer_asset.src_account_id,
                   transfer_asset.description);

        return false;
      }

      AccountAsset dest_AccountAsset;
      auto dest_account_asset = queries.getAccountAsset(
          transfer_asset.dest_account_id, transfer_asset.asset_id);
      auto asset = queries.getAsset(transfer_asset.asset_id);
      if (not asset.has_value()) {
        log_->info("asset {} is absent of {}",
                   transfer_asset.asset_id,
                   transfer_asset.dest_account_id,
                   transfer_asset.description);

        return false;
      }
      // Precision for both wallets
      auto precision = asset.value().precision;
      if (transfer_asset.amount.getPrecision() != precision) {
        log_->info("precision {} is wrong", precision);
        return false;
      }
      // Get src balance
      auto src_balance = src_account_asset.value().balance;
      auto new_src_balance = src_balance - transfer_asset.amount;
      if (not new_src_balance.has_value()) {
        return false;
      }
      src_balance = new_src_balance.value();
      // Set new balance for source account
      src_account_asset.value().balance = src_balance;

      if (not dest_account_asset.has_value()) {
        // This assert is new for this account - create new AccountAsset
        dest_AccountAsset = AccountAsset();
        dest_AccountAsset.asset_id = transfer_asset.asset_id;
        dest_AccountAsset.account_id = transfer_asset.dest_account_id;
        // Set new balance for dest account
        dest_AccountAsset.balance = transfer_asset.amount;

      } else {
        // Account already has such asset
        dest_AccountAsset = dest_account_asset.value();
        // Get balance dest account
        auto dest_balance = dest_account_asset.value().balance;

        auto new_dest_balance = dest_balance + transfer_asset.amount;
        if (not new_dest_balance.has_value()) {
          return false;
        }
        dest_balance = new_dest_balance.value();
        // Set new balance for dest
        dest_AccountAsset.balance = dest_balance;
      }

      return commands.upsertAccountAsset(dest_AccountAsset)
          and commands.upsertAccountAsset(src_account_asset.value());
    }

    bool TransferAssetExecutor::hasPermissions(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto transfer_asset = static_cast<const TransferAsset &>(command);

      return

          (
              // 1. Creator has granted permission on src_account_id
              (creator_account_id != transfer_asset.src_account_id
               and queries.hasAccountGrantablePermission(
                       creator_account_id,
                       transfer_asset.src_account_id,
                       can_transfer))
              or
              // 2. Creator transfer from their account
              (creator_account_id == transfer_asset.src_account_id
               and checkAccountRolePermission(
                       creator_account_id, queries, can_transfer)))
          // For both cases, dest_account must have can_receive
          and checkAccountRolePermission(
                  transfer_asset.dest_account_id, queries, can_receive);
    }

    bool TransferAssetExecutor::isValid(const Command &command,
                                        ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
      auto transfer_asset = static_cast<const TransferAsset &>(command);

      if (transfer_asset.amount.getIntValue() == 0) {
        log_->info("amount must be not zero");
        return false;
      }

      auto asset = queries.getAsset(transfer_asset.asset_id);
      if (not asset.has_value()) {
        log_->info("Asset not found");
        return false;
      }
      // Amount is formed wrong
      if (transfer_asset.amount.getPrecision() != asset.value().precision) {
        log_->info("Wrong precision");
        return false;
      }
      auto account_asset = queries.getAccountAsset(
          transfer_asset.src_account_id, transfer_asset.asset_id);

      return account_asset.has_value() and
          // Check if dest account exist
          queries.getAccount(transfer_asset.dest_account_id) and
          // Balance in your wallet should be at least amount of transfer
          account_asset.value().balance >= transfer_asset.amount;
    }

  }  // namespace model
}  // namespace iroha
