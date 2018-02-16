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
#include <builders/protobuf/common_objects/proto_account_asset_builder.hpp>
#include <builders/protobuf/common_objects/proto_amount_builder.hpp>
#include <builders/protobuf/common_objects/proto_asset_builder.hpp>
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

  CommandExecutor::CommandExecutor(
      std::shared_ptr<iroha::ametsuchi::WsvQuery> queries,
      std::shared_ptr<iroha::ametsuchi::WsvCommand> commands)
      : queries(queries), commands(commands) {}

  void CommandExecutor::setCreatorAccountId(std::string creator_account_id) {
    this->creator_account_id = creator_account_id;
  }

  /**
   * Sums up two optionals of the amounts.
   * Requires to have the same scale.
   * Otherwise nullopt is returned
   * @param a left term
   * @param b right term
   * @param optional result
   */
  boost::optional<shared_model::proto::Amount> operator+(
      const shared_model::interface::Amount &a,
      const shared_model::interface::Amount &b) {
    // check precisions
    if (a.precision() != b.precision()) {
      return boost::none;
    }
    shared_model::proto::AmountBuilder amount_builder;
    auto res = amount_builder.precision(a.precision())
                   .intValue(a.intValue() + b.intValue())
                   .build();
    // check overflow
    if (res.intValue() < a.intValue() or res.intValue() < b.intValue()) {
      return boost::none;
    }
    return boost::optional<shared_model::proto::Amount>(res);
  }

  /**
   * Subtracts two optionals of the amounts.
   * Requires to have the same scale.
   * Otherwise nullopt is returned
   * @param a left term
   * @param b right term
   * @param optional result
   */
  boost::optional<shared_model::proto::Amount> operator-(
      const shared_model::interface::Amount &a,
      const shared_model::interface::Amount &b) {
    // check precisions
    if (a.precision() != b.precision()) {
      return boost::none;
    }
    // check if a greater than b
    if (a.intValue() < b.intValue()) {
      return boost::none;
    }
    shared_model::proto::AmountBuilder amount_builder;
    auto res = amount_builder.precision(a.precision())
                   .intValue(a.intValue() - b.intValue())
                   .build();
    return boost::optional<shared_model::proto::Amount>(res);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::AddAssetQuantity> &command) {
    std::string command_name = "AddAssetQuantity";
    auto asset_old = queries->getAsset(command->assetId());  // Old model
    if (not asset_old.has_value()) {
      return makeExecutionError(
          (boost::format("asset %s is absent") % command->assetId()).str(),
          command_name);
    }
    auto asset = shared_model::proto::from_old(asset_old.value());
    auto precision = asset.precision();

    if (command->amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision mismatch: expected %d, but got %d")
           % precision % command->amount().precision())
              .str(),
          command_name);
    }

    if (not queries->getAccount(command->accountId())
                .has_value()) {  // Old model
      return makeExecutionError(
          (boost::format("account %s is absent") % command->accountId()).str(),
          command_name);
    }
    auto account_asset_old = queries->getAccountAsset(
        command->accountId(), command->assetId());  // Old model

    shared_model::proto::Amount new_balance =
        amount_builder.precision(command->amount().precision())
            .intValue(command->amount().intValue())
            .build();

    if (account_asset_old.has_value()) {
      auto account_asset =
          shared_model::proto::from_old(account_asset_old.value());
      auto balance = new_balance + account_asset.balance();
      if (not balance) {
        return makeExecutionError("amount overflows balance", command_name);
      }

      auto account_asset_new = account_asset_builder.balance(balance.value())
                                   .accountId(command->accountId())
                                   .assetId(command->assetId())
                                   .build();
      return errorIfNot(commands->upsertAccountAsset(account_asset_new),
                        "failed to update account asset",
                        command_name);
    }

    auto account_asset = account_asset_builder.balance(new_balance)
                             .accountId(command->accountId())
                             .assetId(command->assetId())
                             .build();
    return errorIfNot(commands->upsertAccountAsset(account_asset),
                      "failed to update account asset",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::AddPeer> &command) {
    return errorIfNot(
        commands->insertPeer(command->peer()), "peer is not unique", "AddPeer");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::AddSignatory> &command) {
    std::string command_name = "AddSignatory";
    if (not commands->insertSignatory(command->pubkey())) {
      return makeExecutionError("failed to insert signatory", command_name);
    }
    return errorIfNot(commands->insertAccountSignatory(command->accountId(),
                                                       command->pubkey()),
                      "failed to insert account signatory",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::AppendRole> &command) {
    return errorIfNot(
        commands->insertAccountRole(command->accountId(), command->roleName()),
        "failed to insert account role",
        "AppendRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::CreateAccount> &command) {
    std::string command_name = "CreateAccount";
    auto account =
        account_builder
            .accountId(command->accountName() + "@" + command->domainId())
            .domainId(command->domainId())
            .quorum(1)
            .jsonData("{}")
            .build();
    auto domain = queries->getDomain(command->domainId());  // Old model
    if (not domain.has_value()) {
      return makeExecutionError(
          (boost::format("Domain %s not found") % command->domainId()).str(),
          command_name);
    }
    std::string domain_default_role = domain.value().default_role;
    // TODO: remove insert signatory from here ?
    if (not commands->insertSignatory(command->pubkey())) {
      return makeExecutionError("failed to insert signatory", command_name);
    }
    if (not commands->insertAccount(account)) {
      return makeExecutionError("failed to insert account", command_name);
    }
    if (not commands->insertAccountSignatory(account.accountId(),
                                             command->pubkey())) {
      return makeExecutionError("failed to insert account signatory",
                                command_name);
    }
    return errorIfNot(
        commands->insertAccountRole(account.accountId(), domain_default_role),
        "failed to insert account role",
        command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::CreateAsset> &command) {
    auto new_asset =
        asset_builder.assetId(command->assetName() + "#" + command->domainId())
            .domainId(command->domainId())
            .precision(command->precision())
            .build();
    // The insert will fail if asset already exists
    return errorIfNot(commands->insertAsset(new_asset),
                      "failed to insert asset",
                      "CreateAsset");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::CreateDomain> &command) {
    auto new_domain = domain_builder.domainId(command->domainId())
                          .defaultRole(command->userDefaultRole())
                          .build();
    // The insert will fail if domain already exist
    return errorIfNot(commands->insertDomain(new_domain),
                      "failed to insert domain",
                      "CreateDomain");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::CreateRole> &command) {
    std::string command_name = "CreateRole";
    if (not commands->insertRole(command->roleName())) {
      return makeExecutionError("failed to insert role: " + command->roleName(),
                                command_name);
    }

    return errorIfNot(commands->insertRolePermissions(
                          command->roleName(), command->rolePermissions()),
                      "failed to insert role permissions",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::DetachRole> &command) {
    return errorIfNot(
        commands->deleteAccountRole(command->accountId(), command->roleName()),
        "failed to delete account role",
        "DetachRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::GrantPermission> &command) {
    return errorIfNot(
        commands->insertAccountGrantablePermission(command->accountId(),
                                                   creator_account_id,
                                                   command->permissionName()),
        "failed to insert account grantable permission",
        "GrantPermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::RemoveSignatory> &command) {
    std::string command_name = "RemoveSignatory";

    // Delete will fail if account signatory doesn't exist
    if (not commands->deleteAccountSignatory(command->accountId(),
                                             command->pubkey())) {
      return makeExecutionError("failed to delete account signatory",
                                command_name);
    }
    return errorIfNot(commands->deleteSignatory(command->pubkey()),
                      "failed to delete signatory",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::RevokePermission> &command) {
    return errorIfNot(
        commands->deleteAccountGrantablePermission(command->accountId(),
                                                   creator_account_id,
                                                   command->permissionName()),
        "failed to delete account grantable permision",
        "RevokePermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::SetAccountDetail> &command) {
    auto creator = creator_account_id;
    if (creator_account_id.empty()) {
      // When creator is not known, it is genesis block
      creator = "genesis";
    }
    return errorIfNot(
        commands->setAccountKV(
            command->accountId(), creator, command->key(), command->value()),
        "failed to set account key-value",
        "SetAccountDetail");
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::SetQuorum> &command) {
    std::string command_name = "SetQuorum";

    auto account_old = queries->getAccount(command->accountId());  // Old model
    if (not account_old.has_value()) {
      return makeExecutionError(
          (boost::format("absent account %s") % command->accountId()).str(),
          command_name);
    }
    auto account = account_builder.domainId(account_old.value().domain_id)
                       .accountId(account_old.value().account_id)
                       .jsonData(account_old.value().json_data)
                       .quorum(command->newQuorum())
                       .build();
    return errorIfNot(commands->updateAccount(account),
                      "failed to update account",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::SubtractAssetQuantity>
          &command) {
    std::string command_name = "SubtractAssetQuantity";
    auto asset_old = queries->getAsset(command->assetId());  // Old model
    if (not asset_old) {
      return makeExecutionError(
          (boost::format("asset %s is absent") % command->assetId()).str(),
          command_name);
    }
    auto asset = shared_model::proto::from_old(asset_old.value());
    auto precision = asset.precision();

    if (command->amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision mismatch: expected %d, but got %d")
           % precision % command->amount().precision())
              .str(),
          command_name);
    }
    auto account_asset_old = queries->getAccountAsset(
        command->accountId(), command->assetId());  // Old model
    if (not account_asset_old.has_value()) {
      return makeExecutionError((boost::format("%s do not have %s")
                                 % command->accountId() % command->assetId())
                                    .str(),
                                command_name);
    }

    auto account_asset =
        shared_model::proto::from_old(account_asset_old.value());

    auto new_balance = account_asset.balance() - command->amount();
    if (not new_balance) {
      return makeExecutionError("Not sufficient amount", command_name);
    }
    auto account_asset_new = account_asset_builder.balance(*new_balance)
                                 .accountId(account_asset.accountId())
                                 .assetId(account_asset.assetId())
                                 .build();
    return errorIfNot(commands->upsertAccountAsset(account_asset_new),
                      "Failed to upsert account asset",
                      command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const detail::PolymorphicWrapper<interface::TransferAsset> &command) {
    std::string command_name = "TransferAsset";

    auto src_account_asset_old =
        queries->getAccountAsset(command->srcAccountId(), command->assetId());
    if (not src_account_asset_old.has_value()) {
      return makeExecutionError((boost::format("asset %s is absent of %s")
                                 % command->assetId() % command->srcAccountId())
                                    .str(),
                                command_name);
    }
    auto src_account_asset =
        shared_model::proto::from_old(src_account_asset_old.value());
    iroha::model::AccountAsset dest_AccountAsset;
    auto dest_account_asset_old =
        queries->getAccountAsset(command->destAccountId(), command->assetId());
    auto asset_old = queries->getAsset(command->assetId());
    if (not asset_old.has_value()) {
      return makeExecutionError(
          (boost::format("asset %s is absent of %s") % command->assetId()
           % command->destAccountId())
              .str(),
          command_name);
    }
    auto asset = shared_model::proto::from_old(asset_old.value());
    // Precision for both wallets
    auto precision = asset.precision();
    if (command->amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision %d is wrong") % precision).str(),
          command_name);
    }
    // Get src balance
    if (command->amount().precision() != src_account_asset.balance().precision()
        or command->amount().intValue()
            > src_account_asset.balance().intValue()) {
      return makeExecutionError("not enough assets on source account",
                                command_name);
    }
    auto new_src_balance =
        amount_builder.precision(command->amount().precision())
            .intValue(src_account_asset.balance().intValue()
                      - command->amount().intValue())
            .build();
    // Set new balance for source account
    auto src_account_asset_new =
        account_asset_builder.assetId(src_account_asset.assetId())
            .accountId(src_account_asset.accountId())
            .balance(new_src_balance)
            .build();

    if (not dest_account_asset_old.has_value()) {
      // This assert is new for this account - create new AccountAsset
      dest_AccountAsset = iroha::model::AccountAsset();
      dest_AccountAsset.asset_id = command->assetId();
      dest_AccountAsset.account_id = command->destAccountId();
      // Set new balance for dest account
      dest_AccountAsset.balance =
          *std::shared_ptr<iroha::Amount>(command->amount().makeOldModel());
      auto dest_account_asset_new = shared_model::proto::from_old(dest_AccountAsset);

      if (not commands->upsertAccountAsset(dest_account_asset_new)) {
        return makeExecutionError("failed to upsert destination balance",
                                  command_name);
      }
      return errorIfNot(commands->upsertAccountAsset(src_account_asset_new),
                        "failed to upsert source account",
                        command_name);
    } else {
      auto dest_account_asset =
          shared_model::proto::from_old(dest_account_asset_old.value());
      auto new_dest_balance = dest_account_asset.balance() + command->amount();
      if (not new_dest_balance) {
        return makeExecutionError("operation overflows destination balance",
                                  command_name);
      }
      auto dest_account_asset_new =
          account_asset_builder.assetId(command->assetId())
              .accountId(command->destAccountId())
              .balance(new_dest_balance.get())
              .build();
      if (not commands->upsertAccountAsset(dest_account_asset_new)) {
        return makeExecutionError("failed to upsert destination balance",
                                  command_name);
      }
      return errorIfNot(commands->upsertAccountAsset(src_account_asset_new),
                        "failed to upsert source account",
                        command_name);
    }

    //    if (not commands->upsertAccountAsset(
    //            shared_model::proto::from_old(dest_AccountAsset))) {
    //      return makeExecutionError("failed to upsert destination balance",
    //                                command_name);
    //    }
    //    return errorIfNot(commands->upsertAccountAsset(src_account_asset_new),
    //                      "failed to upsert source account",
    //                      command_name);
  }

  // ----------------------| Validator |----------------------

  CommandValidator::CommandValidator(
      std::shared_ptr<iroha::ametsuchi::WsvQuery> queries)
      : queries(queries) {}

  void CommandValidator::setCreatorAccountId(std::string creator_account_id) {
    this->creator_account_id = creator_account_id;
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
    auto account_old = queries.getAccount(command.accountId());  // Old model
    auto signatories =
        queries.getSignatories(command.accountId());  // Old model

    if (not(account_old.has_value() and signatories.has_value())) {
      // No account or signatories found
      return false;
    }
      auto account = shared_model::proto::from_old(account_old.value());

    auto newSignatoriesSize = signatories.value().size() - 1;

    // You can't remove if size of rest signatories less than the quorum
    return newSignatoriesSize >= account.quorum();
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
    auto signatories =
        queries.getSignatories(command.accountId());  // Old model

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

    auto asset_old = queries.getAsset(command.assetId());  // Old model
    if (not asset_old.has_value()) {
      return false;
    }
    auto asset = shared_model::proto::from_old(asset_old.value());
    // Amount is formed wrong
    if (command.amount().precision() != asset.precision()) {
      return false;
    }
    auto account_asset = queries.getAccountAsset(
        command.srcAccountId(), command.assetId());  // Old model

    return account_asset.has_value()
        // Check if dest account exist
        and queries.getAccount(command.destAccountId()) and
        // Balance in your wallet should be at least amount of transfer
        account_asset.value().balance
        >= *std::shared_ptr<iroha::Amount>(command.amount().makeOldModel());
  }
}  // namespace shared_model