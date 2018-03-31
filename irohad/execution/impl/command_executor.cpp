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

#include "execution/command_executor.hpp"

#include "execution/common_executor.hpp"
#include "interfaces/commands/command.hpp"
#include "validators/permissions.hpp"
#include "utils/amount_utils.hpp"

namespace iroha {

  expected::Error<ExecutionError> makeExecutionError(
      const std::string &error_message,
      const std::string command_name) noexcept {
    return expected::makeError(ExecutionError{command_name, error_message});
  }

  ExecutionResult makeExecutionResult(const ametsuchi::WsvCommandResult &result,
                                      std::string command_name) noexcept {
    return result.match(
        [](const expected::Value<void> &v) -> ExecutionResult { return {}; },
        [&command_name](
            const expected::Error<ametsuchi::WsvError> &e) -> ExecutionResult {
          return expected::makeError(ExecutionError{command_name, e.error});
        });
  }

  CommandExecutor::CommandExecutor(
      std::shared_ptr<ametsuchi::WsvQuery> queries,
      std::shared_ptr<ametsuchi::WsvCommand> commands)
      : queries(queries), commands(commands) {}

  void CommandExecutor::setCreatorAccountId(
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    this->creator_account_id = creator_account_id;
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::AddAssetQuantity> &command) {
    std::string command_name = "AddAssetQuantity";
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

    if (not queries->getAccount(command->accountId())) {
      return makeExecutionError(
          (boost::format("account %s is absent") % command->accountId()).str(),
          command_name);
    }
    auto account_asset =
        queries->getAccountAsset(command->accountId(), command->assetId());

    auto new_balance = amount_builder_.precision(command->amount().precision())
                           .intValue(command->amount().intValue())
                           .build();
    using AccountAssetResult =
        expected::Result<std::shared_ptr<shared_model::interface::AccountAsset>,
                         iroha::ExecutionError>;
    auto account_asset_new = new_balance.match(
        [this, &account_asset, &command_name, &command](
            const expected::Value<
                std::shared_ptr<shared_model::interface::Amount>>
                &new_balance_val) -> AccountAssetResult {
          expected::PolymorphicResult<shared_model::interface::AccountAsset,
                                      std::string>
              result;
          if (account_asset) {
            result = (*new_balance_val.value + account_asset.value()->balance())
                | [this, &command](const auto &balance) {
                    return account_asset_builder_.balance(*balance)
                        .accountId(command->accountId())
                        .assetId(command->assetId())
                        .build();
                  };
          } else {
            result = account_asset_builder_.balance(*new_balance_val.value)
                         .accountId(command->accountId())
                         .assetId(command->assetId())
                         .build();
          }
          return result.match(
              [](expected::Value<
                  std::shared_ptr<shared_model::interface::AccountAsset>>
                     &new_account_asset_val) -> AccountAssetResult {
                return expected::makeValue(new_account_asset_val.value);
              },
              [&command_name](const auto &error) -> AccountAssetResult {
                return makeExecutionError(*error.error, command_name);
              });
        },
        [&command_name](const auto &error) -> AccountAssetResult {
          return makeExecutionError(
              "amount builder failed. reason " + *error.error, command_name);
        });

    return account_asset_new.match(
        [&](const expected::Value<
            std::shared_ptr<shared_model::interface::AccountAsset>>
                &account_asset_new_val) -> ExecutionResult {
          return makeExecutionResult(
              commands->upsertAccountAsset(*account_asset_new_val.value),
              command_name);
        },
        [&command_name](const auto &account_asset_error) -> ExecutionResult {
          return makeExecutionError("account asset builder failed. reason "
                                        + account_asset_error.error.toString(),
                                    command_name);
        });
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::AddPeer> &command) {
    return makeExecutionResult(commands->insertPeer(command->peer()),
                               "AddPeer");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::AddSignatory> &command) {
    auto result = commands->insertSignatory(command->pubkey()) | [&] {
      return commands->insertAccountSignatory(command->accountId(),
                                              command->pubkey());
    };
    return makeExecutionResult(result, "AddSignatory");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::AppendRole> &command) {
    return makeExecutionResult(
        commands->insertAccountRole(command->accountId(), command->roleName()),
        "AppendRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::CreateAccount> &command) {
    std::string command_name = "CreateAccount";
    auto account =
        account_builder_
            .accountId(command->accountName() + "@" + command->domainId())
            .domainId(command->domainId())
            .quorum(1)
            .jsonData("{}")
            .build();
    return account.match(
        [&](const expected::Value<
            std::shared_ptr<shared_model::interface::Account>> &account_val)
            -> ExecutionResult {
          auto domain = queries->getDomain(command->domainId());
          if (not domain) {
            return makeExecutionError(
                (boost::format("Domain %s not found") % command->domainId())
                    .str(),
                command_name);
          }
          std::string domain_default_role = domain.value()->defaultRole();
          // Account must have unique initial pubkey
          auto result = commands->insertSignatory(command->pubkey()) | [&] {
            return commands->insertAccount(*account_val.value);
          } | [&] {
            return commands->insertAccountSignatory(
                (*account_val.value).accountId(), command->pubkey());
          } | [&] {
            return commands->insertAccountRole((*account_val.value).accountId(),
                                               domain_default_role);
          };
          return makeExecutionResult(result, command_name);
        },
        [&command_name](const auto &error) -> ExecutionResult {
          return makeExecutionError(
              "account builder failed. reason " + *error.error, command_name);
        });
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::CreateAsset> &command) {
    std::string command_name = "CreateAsset";
    auto new_asset =
        asset_builder_.assetId(command->assetName() + "#" + command->domainId())
            .domainId(command->domainId())
            .precision(command->precision())
            .build();
    return new_asset.match(
        [&](const expected::Value<
            std::shared_ptr<shared_model::interface::Asset>> &new_asset_val)
            -> ExecutionResult {
          // The insert will fail if asset already exists
          return makeExecutionResult(
              commands->insertAsset(*new_asset_val.value), command_name);
        },
        [&command_name](const auto &error) -> ExecutionResult {
          return makeExecutionError(
              "asset builder failed. reason " + *error.error, command_name);
        });
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::CreateDomain> &command) {
    std::string command_name = "CreateDomain";
    auto new_domain = domain_builder_.domainId(command->domainId())
                          .defaultRole(command->userDefaultRole())
                          .build();
    return new_domain.match(
        [&](const expected::Value<
            std::shared_ptr<shared_model::interface::Domain>> &new_domain_val)
            -> ExecutionResult {
          // The insert will fail if domain already exist
          return makeExecutionResult(
              commands->insertDomain(*new_domain_val.value), command_name);
        },
        [&command_name](const auto &error) -> ExecutionResult {
          return makeExecutionError(
              "domain builder failed. reason " + *error.error, command_name);
        });
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::CreateRole> &command) {
    std::string command_name = "CreateRole";
    auto result = commands->insertRole(command->roleName()) | [&] {
      return commands->insertRolePermissions(command->roleName(),
                                             command->rolePermissions());
    };
    return makeExecutionResult(result, command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::DetachRole> &command) {
    return makeExecutionResult(
        commands->deleteAccountRole(command->accountId(), command->roleName()),
        "DetachRole");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::GrantPermission> &command) {
    return makeExecutionResult(
        commands->insertAccountGrantablePermission(command->accountId(),
                                                   creator_account_id,
                                                   command->permissionName()),
        "GrantPermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::RemoveSignatory> &command) {
    std::string command_name = "RemoveSignatory";

    // Delete will fail if account signatory doesn't exist
    auto result = commands->deleteAccountSignatory(command->accountId(),
                                                   command->pubkey())
        | [&] { return commands->deleteSignatory(command->pubkey()); };
    return makeExecutionResult(result, command_name);
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::RevokePermission> &command) {
    return makeExecutionResult(
        commands->deleteAccountGrantablePermission(command->accountId(),
                                                   creator_account_id,
                                                   command->permissionName()),
        "RevokePermission");
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::SetAccountDetail> &command) {
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
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::SetQuorum> &command) {
    std::string command_name = "SetQuorum";

    auto account = queries->getAccount(command->accountId());
    if (not account) {
      return makeExecutionError(
          (boost::format("absent account %s") % command->accountId()).str(),
          command_name);
    }
    auto account_new = account_builder_.domainId(account.value()->domainId())
                           .accountId(account.value()->accountId())
                           .jsonData(account.value()->jsonData())
                           .quorum(command->newQuorum())
                           .build();

    return account_new.match(
        [&](const expected::Value<
            std::shared_ptr<shared_model::interface::Account>> &account_new_val)
            -> ExecutionResult {
          return makeExecutionResult(
              commands->updateAccount(*account_new_val.value), command_name);
        },
        [&command_name](const auto &error) -> ExecutionResult {
          return makeExecutionError(
              "account builder failed. reason " + *error.error, command_name);
        });
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::SubtractAssetQuantity> &command) {
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
        command->accountId(), command->assetId());
    if (not account_asset) {
      return makeExecutionError((boost::format("%s do not have %s")
                                 % command->accountId() % command->assetId())
                                    .str(),
                                command_name);
    }
    auto account_asset_new =
        (account_asset.value()->balance() - command->amount()) |
        [this, &account_asset](const auto &new_balance) {
          return account_asset_builder_.balance(*new_balance)
              .accountId(account_asset.value()->accountId())
              .assetId(account_asset.value()->assetId())
              .build();
        };

    return account_asset_new.match(
        [&](const expected::Value<
            std::shared_ptr<shared_model::interface::AccountAsset>>
                &account_asset_new_val) -> ExecutionResult {
          return makeExecutionResult(
              commands->upsertAccountAsset(*account_asset_new_val.value),
              command_name);
        },
        [&command_name](const auto &error) -> ExecutionResult {
          return makeExecutionError(
              "account asset builder failed. reason " + *error.error,
              command_name);
        });
  }

  ExecutionResult CommandExecutor::operator()(
      const shared_model::detail::PolymorphicWrapper<
          shared_model::interface::TransferAsset> &command) {
    std::string command_name = "TransferAsset";

    auto src_account_asset =
        queries->getAccountAsset(command->srcAccountId(), command->assetId());
    if (not src_account_asset) {
      return makeExecutionError((boost::format("asset %s is absent of %s")
                                 % command->assetId() % command->srcAccountId())
                                    .str(),
                                command_name);
    }
    auto dest_account_asset =
        queries->getAccountAsset(command->destAccountId(), command->assetId());
    auto asset = queries->getAsset(command->assetId());
    if (not asset) {
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
    // Set new balance for source account
    auto src_account_asset_new =
        (src_account_asset.value()->balance() - command->amount()) |
        [this, &src_account_asset](const auto &new_src_balance) {
          return account_asset_builder_
              .assetId(src_account_asset.value()->assetId())
              .accountId(src_account_asset.value()->accountId())
              .balance(*new_src_balance)
              .build();
        };
    return src_account_asset_new.match(
        [&](const expected::Value<
            std::shared_ptr<shared_model::interface::AccountAsset>>
                &src_account_asset_new_val) -> ExecutionResult {
          expected::PolymorphicResult<shared_model::interface::AccountAsset,
                                      std::string>
              dest_account_asset_new;
          if (not dest_account_asset) {
            // This assert is new for this account - create new AccountAsset
            dest_account_asset_new =
                account_asset_builder_.assetId(command->assetId())
                    .accountId(command->destAccountId())
                    .balance(command->amount())
                    .build();

          } else {
            dest_account_asset_new =
                (dest_account_asset.value()->balance() + command->amount()) |
                [this, &command](const auto &new_dest_balance) {
                  return account_asset_builder_.assetId(command->assetId())
                      .accountId(command->destAccountId())
                      .balance(*new_dest_balance)
                      .build();
                };
          }
          return dest_account_asset_new.match(
              [&](const expected::Value<
                  std::shared_ptr<shared_model::interface::AccountAsset>>
                      &dest_account_asset_new_val) -> ExecutionResult {
                auto result = commands->upsertAccountAsset(
                                  *dest_account_asset_new_val.value)
                    | [&] {
                        return commands->upsertAccountAsset(
                            *src_account_asset_new_val.value);
                      };
                return makeExecutionResult(result, command_name);
              },
              [&command_name](const auto &error) -> ExecutionResult {
                return makeExecutionError(
                    "account asset builder failed. reason " + *error.error,
                    command_name);
              });
        },
        [&command_name](const auto &error) -> ExecutionResult {
          return makeExecutionError(
              "account asset builder failed. reason " + *error.error,
              command_name);
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

  bool CommandValidator::hasPermissions(
      const shared_model::interface::AddAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    // Check if creator has MoneyCreator permission.
    // One can only add to his/her account
    // TODO: 03.02.2018 grimadas IR-935, Separate asset creation for distinct
    // asset types, now: anyone having permission "can_add_asset_qty" can add
    // any asset
    return creator_account_id == command.accountId()
        and checkAccountRolePermission(
                creator_account_id, queries, model::can_add_asset_qty);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::AddPeer &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, model::can_add_peer);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::AddSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // Case 1. When command creator wants to add signatory to their
        // account and he has permission CanAddSignatory
        (command.accountId() == creator_account_id
         and checkAccountRolePermission(
                 creator_account_id, queries, model::can_add_signatory))
        or
        // Case 2. Creator has granted permission for it
        (queries.hasAccountGrantablePermission(
            creator_account_id, command.accountId(), model::can_add_signatory));
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::AppendRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, model::can_append_role);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::CreateAccount &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, model::can_create_account);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::CreateAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, model::can_create_asset);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::CreateDomain &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, model::can_create_domain);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::CreateRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, model::can_create_role);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::DetachRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id, queries, model::can_detach_role);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::GrantPermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return checkAccountRolePermission(
        creator_account_id,
        queries,
        model::can_grant + command.permissionName());
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::RemoveSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // 1. Creator removes signatory from their account, and he must have
        // permission on it
        (creator_account_id == command.accountId()
         and checkAccountRolePermission(
                 creator_account_id, queries, model::can_remove_signatory))
        // 2. Creator has granted permission on removal
        or (queries.hasAccountGrantablePermission(creator_account_id,
                                                  command.accountId(),
                                                  model::can_remove_signatory));
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::RevokePermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return queries.hasAccountGrantablePermission(
        command.accountId(), creator_account_id, command.permissionName());
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::SetAccountDetail &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // Case 1. Creator set details for his account
        creator_account_id == command.accountId() or
        // Case 2. Creator has grantable permission to set account key/value
        queries.hasAccountGrantablePermission(
            creator_account_id, command.accountId(), model::can_set_detail);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::SetQuorum &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return
        // 1. Creator set quorum for his account -> must have permission
        (creator_account_id == command.accountId()
         and checkAccountRolePermission(
                 creator_account_id, queries, model::can_set_quorum))
        // 2. Creator has granted permission on it
        or (queries.hasAccountGrantablePermission(
               creator_account_id, command.accountId(), model::can_set_quorum));
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::SubtractAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return creator_account_id == command.accountId()
        and checkAccountRolePermission(
                creator_account_id, queries, model::can_subtract_asset_qty);
  }

  bool CommandValidator::hasPermissions(
      const shared_model::interface::TransferAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return (
               // 1. Creator has granted permission on src_account_id
               (creator_account_id != command.srcAccountId()
                and queries.hasAccountGrantablePermission(
                        creator_account_id,
                        command.srcAccountId(),
                        model::can_transfer))
               or
               // 2. Creator transfer from their account
               (creator_account_id == command.srcAccountId()
                and checkAccountRolePermission(
                        creator_account_id, queries, model::can_transfer)))
        // For both cases, dest_account must have can_receive
        and checkAccountRolePermission(
                command.destAccountId(), queries, model::can_receive);
  }

  bool CommandValidator::isValid(
      const shared_model::interface::AddAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::AddPeer &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::AddSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::AppendRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto role_permissions = queries.getRolePermissions(command.roleName());
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

  bool CommandValidator::isValid(
      const shared_model::interface::CreateAccount &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::CreateAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::CreateDomain &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::CreateRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return std::all_of(
        command.rolePermissions().begin(),
        command.rolePermissions().end(),
        [&queries, &creator_account_id](auto perm) {
          return checkAccountRolePermission(creator_account_id, queries, perm);
        });
  }

  bool CommandValidator::isValid(
      const shared_model::interface::DetachRole &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::GrantPermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::RemoveSignatory &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto account = queries.getAccount(command.accountId());
    auto signatories =
        queries.getSignatories(command.accountId());

    if (not(account and signatories)) {
      // No account or signatories found
      return false;
    }
    auto newSignatoriesSize = signatories.value().size() - 1;

    // You can't remove if size of rest signatories less than the quorum
    return newSignatoriesSize >= account.value()->quorum();
  }

  bool CommandValidator::isValid(
      const shared_model::interface::RevokePermission &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::SetAccountDetail &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::SetQuorum &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto signatories =
        queries.getSignatories(command.accountId());

    if (not(signatories)) {
      // No  signatories of an account found
      return false;
    }
    // You can't remove if size of rest signatories less than the quorum
    return command.newQuorum() > 0 and command.newQuorum() < 10
        and signatories.value().size() >= command.newQuorum();
  }

  bool CommandValidator::isValid(
      const shared_model::interface::SubtractAssetQuantity &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    return true;
  }

  bool CommandValidator::isValid(
      const shared_model::interface::TransferAsset &command,
      ametsuchi::WsvQuery &queries,
      const shared_model::interface::types::AccountIdType &creator_account_id) {
    auto asset = queries.getAsset(command.assetId());
    if (not asset) {
      return false;
    }
    // Amount is formed wrong
    if (command.amount().precision() != asset.value()->precision()) {
      return false;
    }
    auto account_asset =
        queries.getAccountAsset(command.srcAccountId(), command.assetId());
    if (not account_asset) {
      return false;
    }
    // Check if dest account exist
    return queries.getAccount(command.destAccountId()) and
        // Balance in your wallet should be at least amount of transfer
        compareAmount(account_asset.value()->balance(), command.amount()) >= 0;
  }
}  // namespace iroha
