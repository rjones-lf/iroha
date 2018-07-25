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

#include <boost/mpl/contains.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <string>

#include "execution/command_executor.hpp"

#include "backend/protobuf/permissions.hpp"
#include "execution/common_executor.hpp"
#include "interfaces/commands/command.hpp"
#include "utils/amount_utils.hpp"
#include "validators/permissions.hpp"

using namespace shared_model::detail;
using namespace shared_model::interface::permissions;
using namespace shared_model::proto::permissions;
using namespace std::literals::string_literals;

namespace iroha {

  expected::Error<CommandError> makeCommandError(
      const std::string &error_message,
      const std::string &command_name) noexcept {
    return expected::makeError(CommandError{command_name, error_message});
  }

  CommandResult makeCommandResult(const ametsuchi::WsvCommandResult &result,
                                  std::string command_name) noexcept {
    return result.match(
        [](const expected::Value<void> &v) -> CommandResult { return {}; },
        [&command_name](
            const expected::Error<ametsuchi::WsvError> &e) -> CommandResult {
          return expected::makeError(CommandError{command_name, e.error});
        });
  }

  // ----------------------| Validator |----------------------

  CommandValidator::CommandValidator(
      std::shared_ptr<ametsuchi::WsvQuery> queries)
      : queries(queries) {}

  void CommandValidator::setCreatorAccountId(
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    this->creator_account_id = creator_account_id;
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::AddAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "AddAssetQuantity";
    // TODO: 03.02.2018 grimadas IR-935, Separate asset creation for distinct
    // asset types, now: anyone having permission "can_add_asset_qty" can add
    // any asset
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kAddAssetQty)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kAddAssetQty),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::AddPeer &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "AddPeer";
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kAddPeer)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kAddPeer),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::AddSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "AddSignatory";

    auto creator_adds_signatory_to_his_account = [&creator_account_id,
                                                  &command]() {
      return creator_account_id == command.accountId();
    };
    auto creator_has_permission_on_own = [&creator_account_id, &queries]() {
      return checkAccountRolePermission(
          creator_account_id, queries, Role::kAddSignatory);
    };
    auto creator_has_grantable_permission = [&creator_account_id,
                                             &command,
                                             &queries]() {
      return queries.hasAccountGrantablePermission(
          creator_account_id, command.accountId(), Grantable::kAddMySignatory);
    };

    if (creator_adds_signatory_to_his_account()) {
      if (creator_has_permission_on_own()) {
        // 1. Creator adds signatory to his account, and he has permission for
        // it
        return {};
      } else if (creator_has_grantable_permission()) {
        // 2. Creator adds signatory to his account, and he has grantable
        // permission for it
        return {};
      } else {
        return makeCommandError(
            "has permission command validation failed: account "
                + creator_account_id + " does not have permission "
                + toString(Role::kAddSignatory) + " for his own account",
            command_name);
      }
    } else if (creator_has_grantable_permission()) {
      // 3. Creator adds signatory to another account, and he has grantable
      // permission for it
      return {};
    } else {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Grantable::kAddMySignatory) + " for account "
              + command.accountId(),
          command_name);
    }
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::AppendRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "AppendRole";
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kAppendRole)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kAppendRole),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::CreateAccount &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "CreateAccount";
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kCreateAccount)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kCreateAccount),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::CreateAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "CreateAsset";
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kCreateAsset)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kCreateAsset),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::CreateDomain &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "CreateDomain";
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kCreateDomain)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kCreateDomain),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::CreateRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "CreateRole";
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kCreateRole)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kCreateRole),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::DetachRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "DetachRole";
    if (not checkAccountRolePermission(
            creator_account_id, queries, Role::kDetachRole)) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kDetachRole),
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::GrantPermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "GrantPermission";
    if (not checkAccountRolePermission(
            creator_account_id,
            queries,
            shared_model::interface::permissions::permissionFor(
                command.permissionName()))) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have grantable permission "
              + toString(command.permissionName()) + " to grant",
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::RemoveSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "RemoveSignatory";

    auto creator_removes_signatory_from_his_account = [&creator_account_id,
                                                       &command]() {
      return creator_account_id == command.accountId();
    };
    auto creator_has_permission_on_own = [&creator_account_id, &queries]() {
      return checkAccountRolePermission(
          creator_account_id, queries, Role::kRemoveSignatory);
    };
    auto creator_has_grantable_permission =
        [&creator_account_id, &command, &queries]() {
          return queries.hasAccountGrantablePermission(
              creator_account_id,
              command.accountId(),
              Grantable::kRemoveMySignatory);
        };

    if (creator_removes_signatory_from_his_account()) {
      if (creator_has_permission_on_own()) {
        // 1. Creator removes signatory from his account, and he has permission
        // for it
        return {};
      } else if (creator_has_grantable_permission()) {
        // 2. Creator removes signatory from his account, and he has grantable
        // permission for it
        return {};
      } else {
        return makeCommandError(
            "has permission command validation failed: account "
                + creator_account_id + " does not have permission "
                + toString(Role::kRemoveSignatory) + " for his own account",
            command_name);
      }
    } else if (creator_has_grantable_permission()) {
      // 3. Creator removes signatory from another account, and he has grantable
      // permission for it
      return {};
    } else {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Grantable::kRemoveMySignatory) + " for account "
              + command.accountId(),
          command_name);
    }
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::RevokePermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "RevokePermission";
    if (not queries.hasAccountGrantablePermission(command.accountId(),
                                                  creator_account_id,
                                                  command.permissionName())) {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have grantable permission "
              + toString(command.permissionName()) + " to revoke",
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::SetAccountDetail &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "SetAccountDetail";

    if (
        // Case 1. Creator sets details for his account
        creator_account_id == command.accountId()
        // Case 2. Creator has permission to set account key/value
        or checkAccountRolePermission(
               creator_account_id, queries, Role::kSetDetail)
        // Case 3. Creator has grantable permission to set account key/value
        or queries.hasAccountGrantablePermission(
               creator_account_id,
               command.accountId(),
               Grantable::kSetMyAccountDetail)) {
      return {};
    }

    return makeCommandError("has permission command validation failed: account "
                                + creator_account_id
                                + " tries to set details for account "
                                + command.accountId() + ", but has neither "
                                + toString(Role::kSetDetail) + " nor grantable "
                                + toString(Grantable::kSetMyAccountDetail),
                            command_name);
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::SetQuorum &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "SetQuorum";

    auto creator_sets_quorum_for_his_account = [&creator_account_id,
                                                &command]() {
      return creator_account_id == command.accountId();
    };
    auto creator_has_permission_on_own = [&creator_account_id, &queries]() {
      return checkAccountRolePermission(
          creator_account_id, queries, Role::kSetQuorum);
    };
    auto creator_has_grantable_permission =
        [&creator_account_id, &command, &queries]() {
          return queries.hasAccountGrantablePermission(
              creator_account_id, command.accountId(), Grantable::kSetMyQuorum);
        };

    if (creator_sets_quorum_for_his_account()) {
      if (creator_has_permission_on_own()) {
        // 1. Creator has permission quorum for his account
        return {};
      } else if (creator_has_grantable_permission()) {
        // 2. Creator has grantable permission for his account
        return {};
      } else {
        return makeCommandError(
            "has permission command validation failed: account "
                + creator_account_id + " does not have permission "
                + toString(Role::kSetQuorum) + " for his own account",
            command_name);
      }
    } else if (creator_has_grantable_permission()) {
      // 3. Creator has grantable permission for another account
      return {};
    } else {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Grantable::kSetMyQuorum) + " for account "
              + command.accountId(),
          command_name);
    }
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::SubtractAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "SubtractAssetQuantity";
    if (checkAccountRolePermission(
            creator_account_id, queries, Role::kSubtractAssetQty)) {
      return {};
    } else {
      return makeCommandError(
          "has permission command validation failed: account "
              + creator_account_id + " does not have permission "
              + toString(Role::kSubtractAssetQty) + " for his own account",
          command_name);
    }
  }

  CommandResult CommandValidator::hasPermissions(
      const shared_model::interface::TransferAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "TransferAsset";

    auto creator_transfers_from_his_account = [&creator_account_id,
                                               &command]() {
      return creator_account_id == command.srcAccountId();
    };
    auto creator_has_permission_on_own = [&creator_account_id, &queries]() {
      return checkAccountRolePermission(
          creator_account_id, queries, Role::kTransfer);
    };
    auto creator_has_grantable_permission =
        [&creator_account_id, &command, &queries]() {
          return queries.hasAccountGrantablePermission(
              creator_account_id,
              command.srcAccountId(),
              Grantable::kTransferMyAssets);
        };
    auto dest_can_receive = [&command, &queries]() {
      return checkAccountRolePermission(
          command.destAccountId(), queries, Role::kReceive);
    };

    if (dest_can_receive()) {
      if (not creator_transfers_from_his_account()) {
        if (creator_has_grantable_permission()) {
          // 1. Creator has grantable permission on src_account_id
          return {};
        } else {
          return makeCommandError(
              "has permission command validation failed: account "
                  + creator_account_id + " does not have "
                  + toString(Grantable::kTransferMyAssets) + " for account "
                  + command.srcAccountId(),
              command_name);
        }
      } else {
        if (creator_has_permission_on_own()) {
          // 2. Creator transfers from their account
          return {};
        } else {
          return makeCommandError(
              "has permission command validation failed: account "
                  + creator_account_id + " does not have "
                  + toString(Role::kTransfer) + " for his own account",
              command_name);
        }
      }
    } else {
      // For both cases, dest_account must have can_receive
      return makeCommandError(
          "has permission command validation failed: destination account "
              + command.destAccountId() + " does not have "
              + toString(Role::kReceive),
          command_name);
    }
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::AddAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::AddPeer &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::AddSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::AppendRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "AppendRole";
    auto role_permissions = queries.getRolePermissions(command.roleName());
    auto account_roles = queries.getAccountRoles(creator_account_id);

    if (not role_permissions) {
      return makeCommandError(
          "is valid command validation failed: no permissions in role "
              + command.roleName(),
          command_name);
    }
    if (not account_roles) {
      return makeCommandError(
          "is valid command validation failed: no roles in account "
              + creator_account_id,
          command_name);
    }

    shared_model::interface::RolePermissionSet account_permissions{};
    for (const auto &role : *account_roles) {
      auto permissions = queries.getRolePermissions(role);
      if (not permissions)
        continue;
      account_permissions |= *permissions;
    }

    if (not role_permissions->isSubsetOf(account_permissions)) {
      auto missing_permissions = ""s;
      role_permissions.value().iterate(
          [&account_permissions, &missing_permissions](const auto &perm) {
            if (not account_permissions.test(perm)) {
              missing_permissions += toString(perm) + ", ";
            }
          });

      return makeCommandError(
          "is valid command validation failed: account " + creator_account_id
              + " does not have some of the permissions in a role "
              + command.roleName()
              + " he wants to append; such permissions are "
              + missing_permissions,
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::CreateAccount &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::CreateAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::CreateDomain &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::CreateRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "CreateRole";
    auto set = command.rolePermissions();
    auto missing_permissions = ""s;
    for (size_t i = 0; i < set.size(); ++i) {
      auto perm = static_cast<shared_model::interface::permissions::Role>(i);
      if (set.test(perm)
          and not checkAccountRolePermission(
                  creator_account_id, queries, perm)) {
        missing_permissions += toString(perm) + ", ";
      }
    }
    if (not missing_permissions.empty()) {
      return makeCommandError(
          "is valid command validation failed: account " + creator_account_id
              + " does not have some of the permissions from a role "
              + command.roleName()
              + " he wants to create; such permissions are "
              + missing_permissions,
          command_name);
    }
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::DetachRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::GrantPermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::RemoveSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "RemoveSignatory";
    auto account = queries.getAccount(command.accountId());
    auto signatories = queries.getSignatories(command.accountId());

    if (not account) {
      return makeCommandError("is valid command validation failed: no account "
                                  + command.accountId() + " found",
                              command_name);
    }
    if (not signatories) {
      return makeCommandError(
          "is valid command validation failed: no signatories in account "
              + command.accountId() + " found",
          command_name);
    }

    auto newSignatoriesSize = signatories.value().size() - 1;

    // You can't remove if size of rest signatories less than the quorum
    if (newSignatoriesSize < account.value()->quorum()) {
      return makeCommandError(
          "is valid command validation failed: size of rest signatories "
          "becomes less than the quorum; account id "
              + command.accountId() + ", quorum "
              + std::to_string(account.value()->quorum())
              + ", new desired size " + std::to_string(newSignatoriesSize),
          command_name);
    }

    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::RevokePermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::SetAccountDetail &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::SetQuorum &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "SetQuorum";
    auto signatories = queries.getSignatories(command.accountId());

    if (not(signatories)) {
      // No  signatories of an account found
      return makeCommandError(
          "is valid command validation failed: no signatories of an account "
              + command.accountId() + " found",
          command_name);
    }
    if (command.newQuorum() <= 0 or command.newQuorum() >= 10) {
      return makeCommandError(
          "is valid command validation failed: account's "
                         + command.accountId()
                         + " new quorum size is "
                           "out of bounds; "
                           "value is " + std::to_string(command.newQuorum()),
          command_name);
    }
    // You can't remove if size of rest signatories less than the quorum
    if (signatories.value().size() < command.newQuorum()) {
      return makeCommandError(
          "is valid command validation failed: account's"
                         + command.accountId()
                         + " new quorum size "
                           "is greater than "
                           "the signatories amount; "
                           + std::to_string(command.newQuorum()) +
              " vs " + std::to_string(signatories.value().size()),
          command_name);
    }

    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::SubtractAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return {};
  }

  CommandResult CommandValidator::isValid(
      const shared_model::interface::TransferAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto command_name = "TransferAsset";
    auto asset = queries.getAsset(command.assetId());
    if (not asset) {
      return makeCommandError("is valid command validation failed: account "
                                  + command.srcAccountId()
                                  + ", no asset with such id "
                                  + command.assetId(),
                              command_name);
    }
    // Amount is formed wrong
    if (command.amount().precision() > asset.value()->precision()) {
      return makeCommandError(
               "is valid command validation failed: account "
               + command.srcAccountId()
               + ",  precision of command's "
                 "asset "
                 "amount is greater than the actual asset's one; " + std::to_string(command.amount().precision()) + " vs " + std::to_string(asset.value()->precision()),
          command_name);
    }
    auto account_asset =
        queries.getAccountAsset(command.srcAccountId(), command.assetId());
    if (not account_asset) {
      return makeCommandError(
          "is valid command validation failed: asset " + command.assetId()
              + " does not exist on account " + command.srcAccountId(),
          command_name);
    }
    // Check if dest account exist
    if (not queries.getAccount(command.destAccountId())) {
      return makeCommandError(
          "is valid command validation failed: destination account with id "
              + command.destAccountId() + " does not exist",
          command_name);
    }
    // Balance in your wallet should be at least amount of transfer
    if (compareAmount(account_asset.value()->balance(), command.amount()) < 0) {
      return makeCommandError(
          "is valid command validation failed: not enough "
          "balance on account "
              + command.srcAccountId() + "; transfer amount "
              + command.amount().toStringRepr() + ", balance "
              + account_asset.value()->balance().toStringRepr(),
          command_name);
    }
    return {};
  }
}  // namespace iroha
