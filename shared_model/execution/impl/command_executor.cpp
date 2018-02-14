/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "execution/command_executor.hpp"
#include <boost/mpl/contains.hpp>
#include "backend/protobuf/from_old_model.hpp"
#include "execution/common_executor.hpp"
#include "interfaces/commands/command.hpp"
#include "model/permissions.hpp"
#include "validator/domain_name_validator.hpp"

namespace shared_model {

  iroha::expected::Error<ExecutionError> makeExecutionError(
      const std::string &error_message,
      const std::string command_name) noexcept {
    return iroha::expected::makeError(
        ExecutionError{command_name, error_message});
  }

  ExecutionResult errorIfNot(bool condition,
                             const std::string &error_message,
                             const std::string command_name) noexcept {
    if (not condition) {
      return makeExecutionError(error_message, command_name);
    }
    return {};
  }

  CommandExecutor::CommandExecutor() {}

  ExecutionResult CommandExecutor::operator()(
      const interface::AddAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "AddAssetQuantity";
    auto asset = queries.getAsset(command.assetId());
    if (not asset.has_value()) {
      return makeExecutionError(
          (boost::format("asset %s is absent") % command.assetId()).str(),
          command_name);
    }
    auto precision = asset.value().precision;

    if (command.amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision mismatch: expected %d, but got %d")
           % precision % command.amount().precision())
              .str(),
          command_name);
    }

    if (not queries.getAccount(command.accountId()).has_value()) {
      return makeExecutionError(
          (boost::format("account %s is absent") % command.accountId()).str(),
          command_name);
    }
    auto account_asset =
        queries.getAccountAsset(command.accountId(), command.assetId());
    if (not account_asset.has_value()) {
      account_asset = iroha::model::AccountAsset();
      account_asset->asset_id = command.assetId();
      account_asset->account_id = command.accountId();
      account_asset->balance =
          *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());
    } else {
      auto account_asset_value = account_asset.value();

      auto new_balance = account_asset->balance
          + *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());
      if (not new_balance.has_value()) {
        return makeExecutionError("amount overflows balance", command_name);
      }
      account_asset->balance = new_balance.value();
    }

    return errorIfNot(commands.upsertAccountAsset(
                          shared_model::proto::from_old(account_asset.value())),
                      "failed to update account asset",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::AddPeer &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    return errorIfNot(
        commands.insertPeer(command.peer()), "peer is not unique", "AddPeer");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::AddSignatory &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "AddSignatory";
    if (not commands.insertSignatory(command.pubkey())) {
      return makeExecutionError("failed to insert signatory", command_name);
    }
    return errorIfNot(
        commands.insertAccountSignatory(command.accountId(), command.pubkey()),
        "failed to insert account signatory",
        command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::AppendRole &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    return errorIfNot(
        commands.insertAccountRole(command.accountId(), command.roleName()),
        "failed to insert account role",
        "AppendRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::CreateAccount &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "CreateAccount";

    iroha::model::Account account;
    account.account_id = command.accountName() + "@" + command.domainId();

    account.domain_id = command.domainId();
    account.quorum = 1;
    account.json_data = "{}";
    auto domain = queries.getDomain(command.domainId());
    if (not domain.has_value()) {
      return makeExecutionError(
          (boost::format("Domain %s not found") % command.domainId()).str(),
          command_name);
    }
    // TODO: remove insert signatory from here ?
    if (not commands.insertSignatory(command.pubkey())) {
      return makeExecutionError("failed to insert signatory", command_name);
    }
    shared_model::proto::Account acc = shared_model::proto::from_old(account);
    if (not commands.insertAccount(acc)) {
      return makeExecutionError("failed to insert account", command_name);
    }
    if (not commands.insertAccountSignatory(account.account_id,
                                            command.pubkey())) {
      return makeExecutionError("failed to insert account signatory",
                                command_name);
    }
    return errorIfNot(commands.insertAccountRole(account.account_id,
                                                 domain.value().default_role),
                      "failed to insert account role",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::CreateAsset &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    iroha::model::Asset new_asset;
    new_asset.asset_id = command.assetName() + "#" + command.domainId();
    new_asset.domain_id = command.domainId();
    new_asset.precision = command.precision();
    // The insert will fail if asset already exists
    return errorIfNot(
        commands.insertAsset(shared_model::proto::from_old(new_asset)),
        "failed to insert asset",
        "CreateAsset");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::CreateDomain &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    iroha::model::Domain new_domain;
    new_domain.domain_id = command.domainId();
    new_domain.default_role = command.userDefaultRole();
    // The insert will fail if domain already exist
    return errorIfNot(
        commands.insertDomain(shared_model::proto::from_old(new_domain)),
        "failed to insert domain",
        "CreateDomain");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::CreateRole &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "CreateRole";
    if (not commands.insertRole(command.roleName())) {
      return makeExecutionError("failed to insert role: " + command.roleName(),
                                command_name);
    }

    return errorIfNot(commands.insertRolePermissions(command.roleName(),
                                                     command.rolePermissions()),
                      "failed to insert role permissions",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::DetachRole &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    return errorIfNot(
        commands.deleteAccountRole(command.accountId(), command.roleName()),
        "failed to delete account role",
        "DetachRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::GrantPermission &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    return errorIfNot(
        commands.insertAccountGrantablePermission(
            command.accountId(), creator_account_id, command.permissionName()),
        "failed to insert account grantable permission",
        "GrantPermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::RemoveSignatory &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "RemoveSignatory";

    // Delete will fail if account signatory doesn't exist
    if (not commands.deleteAccountSignatory(command.accountId(),
                                            command.pubkey())) {
      return makeExecutionError("failed to delete account signatory",
                                command_name);
    }
    return errorIfNot(commands.deleteSignatory(command.pubkey()),
                      "failed to delete signatory",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::RevokePermission &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    return errorIfNot(
        commands.deleteAccountGrantablePermission(
            command.accountId(), creator_account_id, command.permissionName()),
        "failed to delete account grantable permision",
        "RevokePermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::SetAccountDetail &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    auto creator = creator_account_id;
    if (creator_account_id.empty()) {
      // When creator is not known, it is genesis block
      creator = "genesis";
    }
    return errorIfNot(
        commands.setAccountKV(
            command.accountId(), creator, command.key(), command.value()),
        "failed to set account key-value",
        "SetAccountDetail");
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::SetQuorum &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "SetQuorum";

    auto account = queries.getAccount(command.accountId());
    if (not account.has_value()) {
      return makeExecutionError(
          (boost::format("absent account %s") % command.accountId()).str(),
          command_name);
    }
    account.value().quorum = command.newQuorum();
    shared_model::proto::Account acc =
        shared_model::proto::from_old(account.value());
    return errorIfNot(
        commands.updateAccount(acc), "failed to update account", command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::SubtractAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "SubtractAssetQuantity";
    auto asset = queries.getAsset(command.assetId());
    if (not asset) {
      return makeExecutionError(
          (boost::format("asset %s is absent") % command.assetId()).str(),
          command_name);
    }
    auto precision = asset.value().precision;

    if (command.amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision mismatch: expected %d, but got %d")
           % precision % command.amount().precision())
              .str(),
          command_name);
    }
    auto account_asset =
        queries.getAccountAsset(command.accountId(), command.assetId());
    if (not account_asset.has_value()) {
      return makeExecutionError((boost::format("%s do not have %s")
                                 % command.accountId() % command.assetId())
                                    .str(),
                                command_name);
    }
    auto account_asset_value = account_asset.value();

    auto new_balance = account_asset_value.balance
        - *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());
    if (not new_balance.has_value()) {
      return makeExecutionError("Not sufficient amount", command_name);
    }
    account_asset->balance = new_balance.value();

    return errorIfNot(commands.upsertAccountAsset(
                          shared_model::proto::from_old(account_asset.value())),
                      "Failed to upsert account asset",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const interface::TransferAsset &command,
      iroha::ametsuchi::WsvQuery &queries,
      iroha::ametsuchi::WsvCommand &commands,
      const std::string &creator_account_id) {
    std::string command_name = "TransferAsset";

    auto src_account_asset =
        queries.getAccountAsset(command.srcAccountId(), command.assetId());
    if (not src_account_asset.has_value()) {
      return makeExecutionError((boost::format("asset %s is absent of %s")
                                 % command.assetId() % command.srcAccountId())
                                    .str(),
                                command_name);
    }

    iroha::model::AccountAsset dest_AccountAsset;
    auto dest_account_asset =
        queries.getAccountAsset(command.destAccountId(), command.assetId());
    auto asset = queries.getAsset(command.assetId());
    if (not asset.has_value()) {
      return makeExecutionError((boost::format("asset %s is absent of %s")
                                 % command.assetId() % command.destAccountId())
                                    .str(),
                                command_name);
    }
    // Precision for both wallets
    auto precision = asset.value().precision;
    if (command.amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision %d is wrong") % precision).str(),
          command_name);
    }
    // Get src balance
    auto src_balance = src_account_asset.value().balance;
    auto new_src_balance = src_balance
        - *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());
    if (not new_src_balance.has_value()) {
      return makeExecutionError("not enough assets on source account",
                                command_name);
    }
    src_balance = new_src_balance.value();
    // Set new balance for source account
    src_account_asset.value().balance = src_balance;

    if (not dest_account_asset.has_value()) {
      // This assert is new for this account - create new AccountAsset
      dest_AccountAsset = iroha::model::AccountAsset();
      dest_AccountAsset.asset_id = command.assetId();
      dest_AccountAsset.account_id = command.destAccountId();
      // Set new balance for dest account
      dest_AccountAsset.balance =
          *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());

    } else {
      // Account already has such asset
      dest_AccountAsset = dest_account_asset.value();
      // Get balance dest account
      auto dest_balance = dest_account_asset.value().balance;

      auto new_dest_balance = dest_balance
          + *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());
      if (not new_dest_balance.has_value()) {
        return makeExecutionError("operation overflows destination balance",
                                  command_name);
      }
      dest_balance = new_dest_balance.value();
      // Set new balance for dest
      dest_AccountAsset.balance = dest_balance;
    }

    if (not commands.upsertAccountAsset(
            shared_model::proto::from_old(dest_AccountAsset))) {
      return makeExecutionError("failed to upsert destination balance",
                                command_name);
    }
    return errorIfNot(commands.upsertAccountAsset(shared_model::proto::from_old(
                          src_account_asset.value())),
                      "failed to upsert source account",
                      command_name);
  }

  // ----------------------| Validator |----------------------
  CommandValidator::CommandValidator() {}

  template <typename CommandType>
  bool CommandValidator::operator()(const CommandType &command,
                                    iroha::ametsuchi::WsvQuery &queries,
                                    const std::string &creator_account_id) {
    BOOST_STATIC_ASSERT(
        boost::mpl::contains<interface::Command::CommandVariantType::types,
                             CommandType>::value);
    return hasPermissions(command, queries, creator_account_id)
        and isValid(command, queries, creator_account_id);
  }

  bool CommandValidator::hasPermissions(
      const interface::AddAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      const std::string &creator_account_id) {
    // Check if creator has MoneyCreator permission.
    // One can only add to his/her account
    // TODO: In future: Separate money creation for distinct assets
    return creator_account_id == command.accountId()
        and checkAccountRolePermission(
                creator_account_id, queries, iroha::model::can_add_asset_qty);
  }

  bool CommandValidator::hasPermissions(const interface::AddPeer &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_add_peer);
  }

  bool CommandValidator::hasPermissions(const interface::AddSignatory &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return
        // Case 1. When command creator wants to add signatory to their
        // account and he has permission CanAddSignatory
        (command.accountId() == creator_account_id
         and checkAccountRolePermission(
                 creator_account_id, queries, iroha::model::can_add_signatory))
        or
        // Case 2. Creator has granted permission for it
        (queries.hasAccountGrantablePermission(
            creator_account_id,
            command.accountId(),
            iroha::model::can_add_signatory));
  }

  bool CommandValidator::hasPermissions(const interface::AppendRole &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_append_role);
  }

  bool CommandValidator::hasPermissions(const interface::CreateAccount &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_account);
  }

  bool CommandValidator::hasPermissions(const interface::CreateAsset &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_asset);
  }

  bool CommandValidator::hasPermissions(const interface::CreateDomain &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_domain);
  }

  bool CommandValidator::hasPermissions(const interface::CreateRole &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_role);
  }

  bool CommandValidator::hasPermissions(const interface::DetachRole &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_detach_role);
  }

  bool CommandValidator::hasPermissions(
      const interface::GrantPermission &command,
      iroha::ametsuchi::WsvQuery &queries,
      const std::string &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id,
        queries,
        iroha::model::can_grant + command.permissionName());
  }

  bool CommandValidator::hasPermissions(
      const interface::RemoveSignatory &command,
      iroha::ametsuchi::WsvQuery &queries,
      const std::string &creator_account_id) {
    return
        // 1. Creator removes signatory from their account, and he must have
        // permission on it
        (creator_account_id == command.accountId()
         and checkAccountRolePermission(creator_account_id,
                                        queries,
                                        iroha::model::can_remove_signatory))
        // 2. Creator has granted permission on removal
        or (queries.hasAccountGrantablePermission(
               creator_account_id,
               command.accountId(),
               iroha::model::can_remove_signatory));
  }

  bool CommandValidator::hasPermissions(
      const interface::RevokePermission &command,
      iroha::ametsuchi::WsvQuery &queries,
      const std::string &creator_account_id) {
    return queries.hasAccountGrantablePermission(
        command.accountId(), creator_account_id, command.permissionName());
  }

  bool CommandValidator::hasPermissions(
      const interface::SetAccountDetail &command,
      iroha::ametsuchi::WsvQuery &queries,
      const std::string &creator_account_id) {
    return
        // Case 1. Creator set details for his account
        creator_account_id == command.accountId() or
        // Case 2. Creator has grantable permission to set account key/value
        queries.hasAccountGrantablePermission(creator_account_id,
                                              command.accountId(),
                                              iroha::model::can_set_detail);
  }

  bool CommandValidator::hasPermissions(const interface::SetQuorum &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return
        // 1. Creator set quorum for his account -> must have permission
        (creator_account_id == command.accountId()
         and checkAccountRolePermission(
                 creator_account_id, queries, iroha::model::can_set_quorum))
        // 2. Creator has granted permission on it
        or (queries.hasAccountGrantablePermission(
               creator_account_id,
               command.accountId(),
               iroha::model::can_set_quorum));
  }

  bool CommandValidator::hasPermissions(
      const interface::SubtractAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      const std::string &creator_account_id) {
    return creator_account_id == command.accountId()
        and checkAccountRolePermission(creator_account_id,
                                       queries,
                                       iroha::model::can_subtract_asset_qty);
  }

  bool CommandValidator::hasPermissions(const interface::TransferAsset &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const std::string &creator_account_id) {
    return

        (
            // 1. Creator has granted permission on src_account_id
            (creator_account_id != command.srcAccountId()
             and queries.hasAccountGrantablePermission(
                     creator_account_id,
                     command.srcAccountId(),
                     iroha::model::can_transfer))
            or
            // 2. Creator transfer from their account
            (creator_account_id == command.srcAccountId()
             and checkAccountRolePermission(
                     creator_account_id, queries, iroha::model::can_transfer)))
        // For both cases, dest_account must have can_receive
        and checkAccountRolePermission(
                command.destAccountId(), queries, iroha::model::can_receive);
  }

  bool CommandValidator::isValid(const interface::AddAssetQuantity &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const interface::AddPeer &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const interface::AddSignatory &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const interface::AppendRole &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    auto role_permissions = queries.getRolePermissions(command.roleName());
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
                          return account_permissions.find(perm)
                              == account_permissions.end();
                        });
  }

  bool CommandValidator::isValid(const interface::CreateAccount &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return
        // Name is within some range
        not command.accountName().empty()
        // Account must be well-formed (no system symbols)
        and validator::isValidDomainName(command.accountName());
  }

  bool CommandValidator::isValid(const interface::CreateAsset &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return
        // Name is within some range
        not command.assetName().empty() && command.assetName().size() < 10 &&
        // Account must be well-formed (no system symbols)
        std::all_of(std::begin(command.assetName()),
                    std::end(command.assetName()),
                    [](char c) { return std::isalnum(c); });
  }

  bool CommandValidator::isValid(const interface::CreateDomain &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return
        // Name is within some range
        not command.domainId().empty() and command.domainId().size() < 10 and
        // Account must be well-formed (no system symbols)
        std::all_of(std::begin(command.domainId()),
                    std::end(command.domainId()),
                    [](char c) { return std::isalnum(c); });
  }

  bool CommandValidator::isValid(const interface::CreateRole &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    auto role_is_a_subset = std::all_of(
        command.rolePermissions().begin(),
        command.rolePermissions().end(),
        [&queries, &creator_account_id](auto perm) {
          return checkAccountRolePermission(creator_account_id, queries, perm);
        });

    return role_is_a_subset and not command.roleName().empty()
        and command.roleName().size() < 8 and
        // Role must be well-formed (no system symbols)
        std::all_of(std::begin(command.roleName()),
                    std::end(command.roleName()),
                    [](char c) { return std::isalnum(c) and islower(c); });
  }

  bool CommandValidator::isValid(const interface::DetachRole &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const interface::GrantPermission &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    // TODO: no additional checks ?
    return true;
  }

  bool CommandValidator::isValid(const interface::RemoveSignatory &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    auto account = queries.getAccount(command.accountId());
    auto signatories = queries.getSignatories(command.accountId());

    if (not(account.has_value() and signatories.has_value())) {
      // No account or signatories found
      return false;
    }

    auto newSignatoriesSize = signatories.value().size() - 1;

    // You can't remove if size of rest signatories less than the quorum
    return newSignatoriesSize >= account.value().quorum;
  }

  bool CommandValidator::isValid(const interface::RevokePermission &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    // TODO: no checks needed ?
    return true;
  }

  bool CommandValidator::isValid(const interface::SetAccountDetail &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const interface::SetQuorum &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    auto signatories = queries.getSignatories(command.accountId());

    if (not(signatories.has_value())) {
      // No  signatories of an account found
      return false;
    }
    // You can't remove if size of rest signatories less than the quorum
    return command.newQuorum() > 0 and command.newQuorum() < 10
        and signatories.value().size() >= command.newQuorum();
  }

  bool CommandValidator::isValid(
      const interface::SubtractAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      const std::string &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const interface::TransferAsset &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const std::string &creator_account_id) {
    if (command.amount().intValue() == 0) {
      return false;
    }

    auto asset = queries.getAsset(command.assetId());
    if (not asset.has_value()) {
      return false;
    }
    // Amount is formed wrong
    if (command.amount().precision() != asset.value().precision) {
      return false;
    }
    auto account_asset =
        queries.getAccountAsset(command.srcAccountId(), command.assetId());

    return account_asset.has_value()
        // Check if dest account exist
        and queries.getAccount(command.destAccountId()) and
        // Balance in your wallet should be at least amount of transfer
        account_asset.value().balance
        >= *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());
  }
}  // namespace shared_model