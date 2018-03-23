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
#include "model/account.hpp"
#include "model/account_asset.hpp"
#include "model/asset.hpp"
#include "model/commands/all.hpp"
#include "model/domain.hpp"
#include "model/execution/common_executor.hpp"
#include "model/permissions.hpp"
#include "validator/domain_name_validator.hpp"

using namespace std::string_literals;
using namespace iroha::ametsuchi;
using iroha::expected::makeError;

namespace iroha {
  namespace model {
    // ----------------------------| Common |-----------------------------

    bool CommandExecutor::validate(const Command &command,
                                   WsvQuery &queries,
                                   const std::string &creator_account_id) {
      return hasPermissions(command, queries, creator_account_id)
          and isValid(command, queries, creator_account_id);
    }

    ExecutionResult CommandExecutor::makeExecutionResult(
        const ametsuchi::WsvCommandResult &result) const noexcept {
      return result.match(
          [](const expected::Value<void> &v) -> ExecutionResult { return {}; },
          [this](const expected::Error<WsvError> &e) -> ExecutionResult {
            return makeError(ExecutionError{commandName(), e.error});
          });
    }

    ExecutionResult CommandExecutor::makeExecutionResult(
        const std::string &error_message) const noexcept {
      return ExecutionResult(
          expected::makeError(ExecutionError{commandName(), error_message}));
    }

    // ----------------------------| Append Role |-----------------------------
    std::string AppendRoleExecutor::commandName() const noexcept {
      return "AppendRole";
    }

    ExecutionResult AppendRoleExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const AppendRole &>(command);

      return makeExecutionResult(commands.insertAccountRole(
          cmd_value.account_id, cmd_value.role_name));
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

      if (not role_permissions or not account_roles) {
        return false;
      }

      std::set<std::string> account_permissions;
      for (const auto &role : *account_roles) {
        auto permissions = queries.getRolePermissions(role);
        if (not permissions)
          continue;
        for (const auto &permission : *permissions)
          account_permissions.insert(permission);
      }

      return std::none_of((*role_permissions).begin(),
                          (*role_permissions).end(),
                          [&account_permissions](const auto &perm) {
                            return account_permissions.find(perm)
                                == account_permissions.end();
                          });
    }

    // ----------------------------| Detach Role |-----------------------------
    std::string DetachRoleExecutor::commandName() const noexcept {
      return "DetachRole";
    }

    ExecutionResult DetachRoleExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const DetachRole &>(command);

      return makeExecutionResult(commands.deleteAccountRole(
          cmd_value.account_id, cmd_value.role_name));
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
    std::string CreateRoleExecutor::commandName() const noexcept {
      return "CreateRole";
    }

    ExecutionResult CreateRoleExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const CreateRole &>(command);

      auto result = commands.insertRole(cmd_value.role_name) | [&] {
        return commands.insertRolePermissions(cmd_value.role_name,
                                              cmd_value.permissions);
      };
      return makeExecutionResult(result);
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
    std::string GrantPermissionExecutor::commandName() const noexcept {
      return "GrantPermission";
    }

    ExecutionResult GrantPermissionExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const GrantPermission &>(command);
      return makeExecutionResult(commands.insertAccountGrantablePermission(
          cmd_value.account_id, creator_account_id, cmd_value.permission_name));
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
      return true;
    }

    // -----------|Revoke Permission|-----------
    std::string RevokePermissionExecutor::commandName() const noexcept {
      return "RevokePermission";
    }

    ExecutionResult RevokePermissionExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const RevokePermission &>(command);
      return makeExecutionResult(commands.deleteAccountGrantablePermission(
          cmd_value.account_id, creator_account_id, cmd_value.permission_name));
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
      return true;
    }

    // ----------------------------|AddAssetQuantity|-----------------------------
    std::string AddAssetQuantityExecutor::commandName() const noexcept {
      return "AddAssetQuantity";
    }

    ExecutionResult AddAssetQuantityExecutor::execute(
        const Command &command,
        WsvQuery &queries,
        WsvCommand &commands,
        const std::string &creator_account_id) {
      auto add_asset_quantity = static_cast<const AddAssetQuantity &>(command);

      auto asset = queries.getAsset(add_asset_quantity.asset_id);
      if (not asset) {
        return makeExecutionResult(
            (boost::format("asset %s is absent") % add_asset_quantity.asset_id)
                .str());
      }
      auto precision = asset.value()->precision();

      if (add_asset_quantity.amount.getPrecision() != precision) {
        return makeExecutionResult(
            (boost::format("precision mismatch: expected %d, but got %d")
             % precision
             % add_asset_quantity.amount.getPrecision())
                .str());
      }

      if (not queries.getAccount(add_asset_quantity.account_id)) {
        return makeExecutionResult((boost::format("account %s is absent")
                                    % add_asset_quantity.account_id)
                                       .str());
      }

      auto account_asset =
          queries.getAccountAsset(add_asset_quantity.account_id,
                                  add_asset_quantity.asset_id)
          | [](auto &a) {
              return boost::make_optional(
                  *std::unique_ptr<iroha::model::AccountAsset>(
                      a->makeOldModel()));
            };
      if (not account_asset) {
        account_asset = AccountAsset();
        account_asset->asset_id = add_asset_quantity.asset_id;
        account_asset->account_id = add_asset_quantity.account_id;
        account_asset->balance = add_asset_quantity.amount;
      } else {
        auto account_asset_value = account_asset.value();

        auto new_balance =
            account_asset_value.balance + add_asset_quantity.amount;
        if (not new_balance) {
          return makeExecutionResult("amount overflows balance"s);
        }
        account_asset->balance = new_balance.value();
      }

      return makeExecutionResult(
          commands.upsertAccountAsset(account_asset.value()));
    }

    bool AddAssetQuantityExecutor::hasPermissions(
        const Command &command,
        WsvQuery &queries,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const AddAssetQuantity &>(command);
      // Check if creator has MoneyCreator permission.
      // One can only add to his/her account
      // TODO: 03.02.2018 grimadas IR-935, Separate asset creation for distinct
      // asset types, now: anyone having permission "can_add_asset_qty" can add
      // any asset
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
    std::string SubtractAssetQuantityExecutor::commandName() const noexcept {
      return "SubtractAssetQuantity";
    }

    ExecutionResult SubtractAssetQuantityExecutor::execute(
        const Command &command,
        WsvQuery &queries,
        WsvCommand &commands,
        const std::string &creator_account_id) {
      auto subtract_asset_quantity =
          static_cast<const SubtractAssetQuantity &>(command);

      auto asset = queries.getAsset(subtract_asset_quantity.asset_id);
      if (not asset) {
        return makeExecutionResult((boost::format("asset %s is absent")
                                    % subtract_asset_quantity.asset_id)
                                       .str());
      }
      auto precision = asset.value()->precision();

      if (subtract_asset_quantity.amount.getPrecision() != precision) {
        return makeExecutionResult(
            (boost::format("precision mismatch: expected %d, but got %d")
             % precision
             % subtract_asset_quantity.amount.getPrecision())
                .str());
      }
      auto account_asset =
          queries.getAccountAsset(subtract_asset_quantity.account_id,
                                  subtract_asset_quantity.asset_id)
          | [&](auto &a) {
              return boost::make_optional(
                  *std::unique_ptr<iroha::model::AccountAsset>(
                      a->makeOldModel()));
            };
      if (not account_asset) {
        return makeExecutionResult((boost::format("account %s does not have %s")
                                    % subtract_asset_quantity.account_id
                                    % subtract_asset_quantity.asset_id)
                                       .str());
      }
      auto account_asset_value = account_asset.value();

      auto new_balance =
          account_asset_value.balance - subtract_asset_quantity.amount;
      if (not new_balance) {
        return makeExecutionResult("not sufficient amount"s);
      }
      account_asset->balance = new_balance.value();

      return makeExecutionResult(
          commands.upsertAccountAsset(account_asset.value()));
    }

    bool SubtractAssetQuantityExecutor::hasPermissions(
        const Command &command,
        WsvQuery &queries,
        const std::string &creator_account_id) {
      auto cmd_value = static_cast<const SubtractAssetQuantity &>(command);
      return creator_account_id == cmd_value.account_id
          and checkAccountRolePermission(
                  creator_account_id, queries, can_subtract_asset_qty);
    }

    bool SubtractAssetQuantityExecutor::isValid(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      return true;
    }

    // --------------------------|AddPeer|------------------------------
    std::string AddPeerExecutor::commandName() const noexcept {
      return "AddPeer";
    }

    ExecutionResult AddPeerExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto add_peer = static_cast<const AddPeer &>(command);
      // Will return false if peer is not unique
      return makeExecutionResult(commands.insertPeer(add_peer.peer));
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
      return true;
    }

    // -------------------------|AddSignatory|--------------------------
    std::string AddSignatoryExecutor::commandName() const noexcept {
      return "AddSignatory";
    }

    ExecutionResult AddSignatoryExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto add_signatory = static_cast<const AddSignatory &>(command);

      auto result = commands.insertSignatory(add_signatory.pubkey) | [&] {
        return commands.insertAccountSignatory(add_signatory.account_id,
                                               add_signatory.pubkey);
      };

      return makeExecutionResult(result);
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
    std::string CreateAccountExecutor::commandName() const noexcept {
      return "CreateAccount";
    }

    ExecutionResult CreateAccountExecutor::execute(
        const Command &command,
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
      if (not domain) {
        return makeExecutionResult(
            (boost::format("Domain %s not found") % create_account.domain_id)
                .str());
      }
      // Account must have unique initial pubkey
      auto result = commands.insertSignatory(create_account.pubkey) | [&] {
        return commands.insertAccount(account);
      } | [&] {
        return commands.insertAccountSignatory(account.account_id,
                                               create_account.pubkey);
      } | [&] {
        return commands.insertAccountRole(account.account_id,
                                          domain.value()->defaultRole());
      };

      return makeExecutionResult(result);
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
    std::string CreateAssetExecutor::commandName() const noexcept {
      return "CreateAsset";
    }

    ExecutionResult CreateAssetExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto create_asset = static_cast<const CreateAsset &>(command);

      Asset new_asset;
      new_asset.asset_id =
          create_asset.asset_name + "#" + create_asset.domain_id;
      new_asset.domain_id = create_asset.domain_id;
      new_asset.precision = create_asset.precision;
      // The insert will fail if asset already exists
      return makeExecutionResult(commands.insertAsset(new_asset));
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
    std::string CreateDomainExecutor::commandName() const noexcept {
      return "CreateDomain";
    }

    ExecutionResult CreateDomainExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto create_domain = static_cast<const CreateDomain &>(command);

      Domain new_domain;
      new_domain.domain_id = create_domain.domain_id;
      new_domain.default_role = create_domain.user_default_role;
      // The insert will fail if domain already exist
      return makeExecutionResult(commands.insertDomain(new_domain));
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
    std::string RemoveSignatoryExecutor::commandName() const noexcept {
      return "RemoveSignatory";
    }

    ExecutionResult RemoveSignatoryExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto remove_signatory = static_cast<const RemoveSignatory &>(command);

      // Delete will fail if account signatory doesn't exist
      auto result = commands.deleteAccountSignatory(remove_signatory.account_id,
                                                    remove_signatory.pubkey)
          | [&] { return commands.deleteSignatory(remove_signatory.pubkey); };
      return makeExecutionResult(result);
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

    bool RemoveSignatoryExecutor::isValid(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      auto remove_signatory = static_cast<const RemoveSignatory &>(command);

      auto account = queries.getAccount(remove_signatory.account_id);
      auto signatories = queries.getSignatories(remove_signatory.account_id);

      if (not(account and signatories)) {
        // No account or signatories found
        return false;
      }

      auto newSignatoriesSize = signatories.value().size() - 1;

      // You can't remove if size of rest signatories less than the quorum
      return newSignatoriesSize >= account.value()->quorum();
    }

    // -----------------------|SetAccountDetail|-------------------------
    std::string SetAccountDetailExecutor::commandName() const noexcept {
      return "SetAccountDetail";
    }

    ExecutionResult SetAccountDetailExecutor::execute(
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
      return makeExecutionResult(
          commands.setAccountKV(cmd.account_id, creator, cmd.key, cmd.value));
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

    bool SetAccountDetailExecutor::isValid(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        const std::string &creator_account_id) {
      return true;
    }

    // -----------------------|SetQuorum|-------------------------
    std::string SetQuorumExecutor::commandName() const noexcept {
      return "SetQuorum";
    }

    ExecutionResult SetQuorumExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto set_quorum = static_cast<const SetQuorum &>(command);

      auto account = queries.getAccount(set_quorum.account_id) | [](auto &a) {
        return boost::make_optional(
            *std::unique_ptr<iroha::model::Account>(a->makeOldModel()));
      };
      if (not account) {
        return makeExecutionResult(
            (boost::format("absent account %s") % set_quorum.account_id).str());
      }
      account.value().quorum = set_quorum.new_quorum;
      return makeExecutionResult(commands.updateAccount(account.value()));
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
                                    ametsuchi::WsvQuery &queries,
                                    const std::string &creator_account_id) {
      auto set_quorum = static_cast<const SetQuorum &>(command);
      auto signatories = queries.getSignatories(set_quorum.account_id);

      if (not(signatories)) {
        // No  signatories of an account found
        return false;
      }
      // You can't remove if size of rest signatories less than the quorum
      return set_quorum.new_quorum > 0 and set_quorum.new_quorum < 10
          and signatories.value().size() >= set_quorum.new_quorum;
    }

    // ------------------------|TransferAsset|-------------------------
    std::string TransferAssetExecutor::commandName() const noexcept {
      return "TransferAsset";
    }

    ExecutionResult TransferAssetExecutor::execute(
        const Command &command,
        ametsuchi::WsvQuery &queries,
        ametsuchi::WsvCommand &commands,
        const std::string &creator_account_id) {
      auto transfer_asset = static_cast<const TransferAsset &>(command);

      auto src_account_asset =
          queries.getAccountAsset(transfer_asset.src_account_id,
                                  transfer_asset.asset_id)
          | [](auto &a) {
              return boost::make_optional(
                  *std::unique_ptr<iroha::model::AccountAsset>(
                      a->makeOldModel()));
            };
      if (not src_account_asset) {
        return makeExecutionResult((boost::format("asset %s is absent of %s")
                                    % transfer_asset.asset_id
                                    % transfer_asset.src_account_id)
                                       .str());
      }

      AccountAsset dest_AccountAsset;
      auto dest_account_asset =
          queries.getAccountAsset(transfer_asset.dest_account_id,
                                  transfer_asset.asset_id)
          | [](auto &a) {
              return boost::make_optional(
                  *std::unique_ptr<iroha::model::AccountAsset>(
                      a->makeOldModel()));
            };
      auto asset = queries.getAsset(transfer_asset.asset_id);
      if (not asset) {
        return makeExecutionResult((boost::format("asset %s is absent of %s")
                                    % transfer_asset.asset_id
                                    % transfer_asset.dest_account_id)
                                       .str());
      }
      // Precision for both wallets
      auto precision = asset.value()->precision();
      if (transfer_asset.amount.getPrecision() != precision) {
        return makeExecutionResult(
            (boost::format("precision %d is wrong") % precision).str());
      }
      // Get src balance
      auto src_balance = src_account_asset.value().balance;
      auto new_src_balance = src_balance - transfer_asset.amount;
      if (not new_src_balance) {
        return makeExecutionResult(
            (boost::format("not enough assets on source account '%s'")
             % transfer_asset.src_account_id)
                .str());
      }
      src_balance = new_src_balance.value();
      // Set new balance for source account
      src_account_asset.value().balance = src_balance;

      if (not dest_account_asset) {
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
        if (not new_dest_balance) {
          return makeExecutionResult(
              std::string("operation overflows destination balance"));
        }
        dest_balance = new_dest_balance.value();
        // Set new balance for dest
        dest_AccountAsset.balance = dest_balance;
      }

      auto result = commands.upsertAccountAsset(dest_AccountAsset) | [&] {
        return commands.upsertAccountAsset(src_account_asset.value());
      };

      return makeExecutionResult(result);
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
        return false;
      }

      auto asset = queries.getAsset(transfer_asset.asset_id);
      if (not asset) {
        return false;
      }
      // Amount is formed wrong
      if (transfer_asset.amount.getPrecision() != asset.value()->precision()) {
        return false;
      }
      auto account_asset = queries.getAccountAsset(
          transfer_asset.src_account_id, transfer_asset.asset_id);

      return account_asset
          // Check if dest account exist
          and queries.getAccount(transfer_asset.dest_account_id) and
          // Balance in your wallet should be at least amount of transfer
          std::unique_ptr<iroha::model::AccountAsset>(
              account_asset.value()->makeOldModel())
              ->balance
          >= transfer_asset.amount;
    }

  }  // namespace model
}  // namespace iroha
