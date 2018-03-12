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
#include "execution/common_executor.hpp"
#include <boost/mpl/contains.hpp>
#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "builders/protobuf/common_objects/proto_asset_builder.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "interfaces/commands/command.hpp"
#include "model/permissions.hpp"
#include "validator/domain_name_validator.hpp"

namespace iroha {

  iroha::expected::Error<ExecutionError> makeExecutionError(
      const std::string &error_message,
      const std::string command_name) noexcept {
    return iroha::expected::makeError(
        ExecutionError{command_name, error_message});
  }

  ExecutionResult makeExecutionResult(
      const iroha::ametsuchi::WsvCommandResult &result,
      std::string command_name) noexcept {
    return result.match(
        [](const iroha::expected::Value<void> &v) -> ExecutionResult {
          return {};
        },
        [&command_name](
            const iroha::expected::Error<iroha::ametsuchi::WsvError> &e)
            -> ExecutionResult {
          return iroha::expected::makeError(
              ExecutionError{command_name, e.error});
        });
  }

  CommandExecutor::CommandExecutor(
      std::shared_ptr<iroha::ametsuchi::WsvQuery> queries,
      std::shared_ptr<iroha::ametsuchi::WsvCommand> commands)
      : queries(queries), commands(commands) {}

  void CommandExecutor::setCreatorAccountId(shared_model::interface::types::AccountIdType creator_account_id) {
    this->creator_account_id = creator_account_id;
  }

  /**
   * Sums up two amounts.
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
   * Subtracts two amounts.
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

  // to raise to power integer values
  int ipow(int base, int exp) {
    int result = 1;
    while (exp != 0) {
      if (exp & 1) {
        result *= base;
      }
      exp >>= 1;
      base *= base;
    }

    return result;
  }

  int compareAmount(const shared_model::interface::Amount &a,
                    const shared_model::interface::Amount &b) {
    if (a.precision() == b.precision()) {
      return (a.intValue() < b.intValue())
          ? -1
          : (a.intValue() > b.intValue()) ? 1 : 0;
    }
    // when different precisions transform to have the same scale
    auto max_precision = std::max(a.precision(), b.precision());
    auto val1 = a.intValue() * ipow(10, max_precision - a.precision());
    auto val2 = b.intValue() * ipow(10, max_precision - b.precision());
    return (val1 < val2) ? -1 : (val1 > val2) ? 1 : 0;
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::AddAssetQuantity> &command) {
    std::string command_name = "AddAssetQuantity";
    auto asset = queries->getAsset(command->assetId());
    if (not asset.has_value()) {
      return makeExecutionError(
          (boost::format("asset %s is absent") % command->assetId()).str(),
          command_name);
    }
    auto precision = asset.value()->precision();

    if (command->amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision mismatch: expected %d, but got %d")
           % precision % command->amount().precision())
              .str(),
          command_name);
    }

    if (not queries->getAccount(command->accountId())
                .has_value()) {
      return makeExecutionError(
          (boost::format("account %s is absent") % command->accountId()).str(),
          command_name);
    }
    auto account_asset = queries->getAccountAsset(
        command->accountId(), command->assetId());

    shared_model::proto::Amount new_balance =
        amount_builder.precision(command->amount().precision())
            .intValue(command->amount().intValue())
            .build();

    if (account_asset.has_value()) {
      auto balance = new_balance + account_asset.value()->balance();
      if (not balance) {
        return makeExecutionError("amount overflows balance", command_name);
      }

      auto account_asset_new = account_asset_builder.balance(balance.value())
                                   .accountId(command->accountId())
                                   .assetId(command->assetId())
                                   .build();
      return makeExecutionResult(
          commands->upsertAccountAsset(account_asset_new), command_name);
    }

    auto account_asset_new = account_asset_builder.balance(new_balance)
                             .accountId(command->accountId())
                             .assetId(command->assetId())
                             .build();
    return makeExecutionResult(commands->upsertAccountAsset(account_asset_new),
                               command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::AddPeer> &command) {
    return makeExecutionResult(commands->insertPeer(command->peer()),
                               "AddPeer");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::AddSignatory> &command) {
    std::string command_name = "AddSignatory";
    auto result = commands->insertSignatory(command->pubkey()) | [&] {
      return commands->insertAccountSignatory(command->accountId(),
                                              command->pubkey());
    };
    return makeExecutionResult(result, command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::AppendRole> &command) {
    return makeExecutionResult(
        commands->insertAccountRole(command->accountId(), command->roleName()),
        "AppendRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::CreateAccount> &command) {
    std::string command_name = "CreateAccount";
    auto account =
        account_builder
            .accountId(command->accountName() + "@" + command->domainId())
            .domainId(command->domainId())
            .quorum(1)
            .jsonData("{}")
            .build();
    auto domain = queries->getDomain(command->domainId());
    if (not domain.has_value()) {
      return makeExecutionError(
          (boost::format("Domain %s not found") % command->domainId()).str(),
          command_name);
    }
    std::string domain_default_role = domain.value()->defaultRole();
    // TODO: remove insert signatory from here ?
    auto result = commands->insertSignatory(command->pubkey()) | [&] {
      return commands->insertAccount(account);
    } | [&] {
      return commands->insertAccountSignatory(account.accountId(),
                                              command->pubkey());
    } | [&] {
      return commands->insertAccountRole(account.accountId(),
                                         domain_default_role);
    };
    return makeExecutionResult(result, "CreateAccount");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::CreateAsset> &command) {
    auto new_asset =
        asset_builder.assetId(command->assetName() + "#" + command->domainId())
            .domainId(command->domainId())
            .precision(command->precision())
            .build();
    // The insert will fail if asset already exists
    return makeExecutionResult(commands->insertAsset(new_asset), "CreateAsset");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::CreateDomain> &command) {
    auto new_domain = domain_builder.domainId(command->domainId())
                          .defaultRole(command->userDefaultRole())
                          .build();
    // The insert will fail if domain already exist
    return makeExecutionResult(commands->insertDomain(new_domain),
                               "CreateDomain");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::CreateRole> &command) {
    std::string command_name = "CreateRole";
    auto result = commands->insertRole(command->roleName()) | [&] {
      return commands->insertRolePermissions(command->roleName(),
                                             command->rolePermissions());
    };
    return makeExecutionResult(result, command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::DetachRole> &command) {
    return makeExecutionResult(
        commands->deleteAccountRole(command->accountId(), command->roleName()),
        "DetachRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::GrantPermission> &command) {
    return makeExecutionResult(
        commands->insertAccountGrantablePermission(command->accountId(),
                                                   creator_account_id,
                                                   command->permissionName()),
        "GrantPermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::RemoveSignatory> &command) {
    std::string command_name = "RemoveSignatory";

    // Delete will fail if account signatory doesn't exist
    auto result = commands->deleteAccountSignatory(command->accountId(),
                                                   command->pubkey())
        | [&] { return commands->deleteSignatory(command->pubkey()); };
    return makeExecutionResult(result, command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::RevokePermission> &command) {
    return makeExecutionResult(
        commands->deleteAccountGrantablePermission(command->accountId(),
                                                   creator_account_id,
                                                   command->permissionName()),
        "RevokePermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::SetAccountDetail> &command) {
    auto creator = creator_account_id;
    if (creator_account_id.empty()) {
      // When creator is not known, it is genesis block
      creator = "genesis";
    }
    return makeExecutionResult(
        commands->setAccountKV(
            command->accountId(), creator, command->key(), command->value()),
        "SetAccountDetail");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::SetQuorum> &command) {
    std::string command_name = "SetQuorum";

    auto account = queries->getAccount(command->accountId());
    if (not account.has_value()) {
      return makeExecutionError(
          (boost::format("absent account %s") % command->accountId()).str(),
          command_name);
    }
    auto account_new = account_builder.domainId(account.value()->domainId())
                       .accountId(account.value()->accountId())
                       .jsonData(account.value()->jsonData())
                       .quorum(command->newQuorum())
                       .build();
    return makeExecutionResult(commands->updateAccount(account_new), command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::SubtractAssetQuantity>
          &command) {
    std::string command_name = "SubtractAssetQuantity";
    auto asset = queries->getAsset(command->assetId());
    if (not asset) {
      return makeExecutionError(
          (boost::format("asset %s is absent") % command->assetId()).str(),
          command_name);
    }
    auto precision = asset.value()->precision();

    if (command->amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision mismatch: expected %d, but got %d")
           % precision % command->amount().precision())
              .str(),
          command_name);
    }
    auto account_asset = queries->getAccountAsset(
        command->accountId(), command->assetId());  // Old model
    if (not account_asset.has_value()) {
      return makeExecutionError((boost::format("%s do not have %s")
                                 % command->accountId() % command->assetId())
                                    .str(),
                                command_name);
    }


    auto new_balance = account_asset.value()->balance() - command->amount();
    if (not new_balance) {
      return makeExecutionError("Not sufficient amount", command_name);
    }
    auto account_asset_new = account_asset_builder.balance(*new_balance)
                                 .accountId(account_asset.value()->accountId())
                                 .assetId(account_asset.value()->assetId())
                                 .build();
    return makeExecutionResult(commands->upsertAccountAsset(account_asset_new),
                               command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<shared_model::interface::TransferAsset> &command) {
    std::string command_name = "TransferAsset";

    auto src_account_asset =
        queries->getAccountAsset(command->srcAccountId(), command->assetId());
    if (not src_account_asset.has_value()) {
      return makeExecutionError((boost::format("asset %s is absent of %s")
                                 % command->assetId() % command->srcAccountId())
                                    .str(),
                                command_name);
    }
    iroha::model::AccountAsset dest_AccountAsset;
    auto dest_account_asset =
        queries->getAccountAsset(command->destAccountId(), command->assetId());
    auto asset = queries->getAsset(command->assetId());
    if (not asset.has_value()) {
      return makeExecutionError(
          (boost::format("asset %s is absent of %s") % command->assetId()
           % command->destAccountId())
              .str(),
          command_name);
    }
    // Precision for both wallets
    auto precision = asset.value()->precision();
    if (command->amount().precision() != precision) {
      return makeExecutionError(
          (boost::format("precision %d is wrong") % precision).str(),
          command_name);
    }
    // Get src balance
    auto new_src_balance = src_account_asset.value()->balance() - command->amount();
    if (not new_src_balance) {
        return makeExecutionError("not enough assets on source account",
                                command_name);
    }
    // Set new balance for source account
    auto src_account_asset_new =
        account_asset_builder.assetId(src_account_asset.value()->assetId())
            .accountId(src_account_asset.value()->accountId())
            .balance(new_src_balance.get())
            .build();

    if (not dest_account_asset.has_value()) {
      // This assert is new for this account - create new AccountAsset

      auto dest_account_asset_new =
          account_asset_builder.assetId(command->assetId())
              .accountId(command->destAccountId())
              .balance(command->amount())
              .build();
      auto result = commands->upsertAccountAsset(dest_account_asset_new) |
          [&] { return commands->upsertAccountAsset(src_account_asset_new); };
      return makeExecutionResult(result, command_name);
    } else {
      auto new_dest_balance = dest_account_asset.value()->balance() + command->amount();
      if (not new_dest_balance) {
        return makeExecutionError("operation overflows destination balance",
                                  command_name);
      }
      auto dest_account_asset_new =
          account_asset_builder.assetId(command->assetId())
              .accountId(command->destAccountId())
              .balance(new_dest_balance.get())
              .build();
      auto result = commands->upsertAccountAsset(dest_account_asset_new) |
          [&] { return commands->upsertAccountAsset(src_account_asset_new); };
      return makeExecutionResult(result, command_name);
    }
  }

  // ----------------------| Validator |----------------------

  CommandValidator::CommandValidator(
      std::shared_ptr<iroha::ametsuchi::WsvQuery> queries)
      : queries(queries) {}

  void CommandValidator::setCreatorAccountId(shared_model::interface::types::AccountIdType creator_account_id) {
    this->creator_account_id = creator_account_id;
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::AddAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    // Check if creator has MoneyCreator permission.
    // One can only add to his/her account
    // TODO: In future: Separate money creation for distinct assets
    return creator_account_id == command.accountId()
        and checkAccountRolePermission(
                creator_account_id, queries, iroha::model::can_add_asset_qty);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::AddPeer &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_add_peer);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::AddSignatory &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
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

  bool CommandValidator::hasPermissions(const shared_model::interface::AppendRole &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_append_role);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::CreateAccount &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_account);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::CreateAsset &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_asset);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::CreateDomain &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_domain);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::CreateRole &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_create_role);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::DetachRole &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, iroha::model::can_detach_role);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::GrantPermission &command,
      iroha::ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id,
        queries,
        iroha::model::can_grant + command.permissionName());
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::RemoveSignatory &command,
      iroha::ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
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
      const shared_model::interface::RevokePermission &command,
      iroha::ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return queries.hasAccountGrantablePermission(
        command.accountId(), creator_account_id, command.permissionName());
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::SetAccountDetail &command,
      iroha::ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // Case 1. Creator set details for his account
        creator_account_id == command.accountId() or
        // Case 2. Creator has grantable permission to set account key/value
        queries.hasAccountGrantablePermission(creator_account_id,
                                              command.accountId(),
                                              iroha::model::can_set_detail);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::SetQuorum &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
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
      const shared_model::interface::SubtractAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return creator_account_id == command.accountId()
        and checkAccountRolePermission(creator_account_id,
                                       queries,
                                       iroha::model::can_subtract_asset_qty);
  }

  bool CommandValidator::hasPermissions(const shared_model::interface::TransferAsset &command,
                                        iroha::ametsuchi::WsvQuery &queries,
                                        const shared_model::interface::types::AccountIdType &creator_account_id) {
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

  bool CommandValidator::isValid(const shared_model::interface::AddAssetQuantity &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::AddPeer &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::AddSignatory &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::AppendRole &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
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

  bool CommandValidator::isValid(const shared_model::interface::CreateAccount &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // Name is within some range
        not command.accountName().empty()
        // Account must be well-formed (no system symbols)
        and ::validator::isValidDomainName(command.accountName());
  }

  bool CommandValidator::isValid(const shared_model::interface::CreateAsset &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // Name is within some range
        not command.assetName().empty() && command.assetName().size() < 10 &&
        // Account must be well-formed (no system symbols)
        std::all_of(std::begin(command.assetName()),
                    std::end(command.assetName()),
                    [](char c) { return std::isalnum(c); });
  }

  bool CommandValidator::isValid(const shared_model::interface::CreateDomain &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // Name is within some range
        not command.domainId().empty() and command.domainId().size() < 10 and
        // Account must be well-formed (no system symbols)
        std::all_of(std::begin(command.domainId()),
                    std::end(command.domainId()),
                    [](char c) { return std::isalnum(c); });
  }

  bool CommandValidator::isValid(const shared_model::interface::CreateRole &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
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

  bool CommandValidator::isValid(const shared_model::interface::DetachRole &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::GrantPermission &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    // TODO: no additional checks ?
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::RemoveSignatory &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto account = queries.getAccount(command.accountId());
    auto signatories =
        queries.getSignatories(command.accountId());  // Old model

    if (not(account.has_value() and signatories.has_value())) {
      // No account or signatories found
      return false;
    }
    auto newSignatoriesSize = signatories.value().size() - 1;

    // You can't remove if size of rest signatories less than the quorum
    return newSignatoriesSize >= account.value()-> quorum();
  }

  bool CommandValidator::isValid(const shared_model::interface::RevokePermission &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    // TODO: no checks needed ?
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::SetAccountDetail &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::SetQuorum &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
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
      const shared_model::interface::SubtractAssetQuantity &command,
      iroha::ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(const shared_model::interface::TransferAsset &command,
                                 iroha::ametsuchi::WsvQuery &queries,
                                 const shared_model::interface::types::AccountIdType &creator_account_id) {
    if (command.amount().intValue() == 0) {
      return false;
    }

    auto asset = queries.getAsset(command.assetId());
    if (not asset.has_value()) {
      return false;
    }
    // Amount is formed wrong
    if (command.amount().precision() != asset.value()->precision()) {
      return false;
    }
    auto account_asset = queries.getAccountAsset(
        command.srcAccountId(), command.assetId());
    if (not account_asset.has_value()) {
      return false;
    }
    // Check if dest account exist
    return queries.getAccount(command.destAccountId()) and
        // Balance in your wallet should be at least amount of transfer
        compareAmount(account_asset.value()->balance(), command.amount()) >= 0;
  }
}  // namespace shared_model