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

#ifndef IROHA_COMMAND_EXECUTOR_HPP
#define IROHA_COMMAND_EXECUTOR_HPP

#include <boost/format.hpp>
#include <boost/variant/static_visitor.hpp>

#include "ametsuchi/command_executor.hpp"
#include "ametsuchi/wsv_command.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "builders/default_builders.hpp"
#include "common/result.hpp"
#include "interfaces/commands/add_asset_quantity.hpp"
#include "interfaces/commands/add_peer.hpp"
#include "interfaces/commands/add_signatory.hpp"
#include "interfaces/commands/append_role.hpp"
#include "interfaces/commands/create_account.hpp"
#include "interfaces/commands/create_asset.hpp"
#include "interfaces/commands/create_domain.hpp"
#include "interfaces/commands/create_role.hpp"
#include "interfaces/commands/detach_role.hpp"
#include "interfaces/commands/grant_permission.hpp"
#include "interfaces/commands/remove_signatory.hpp"
#include "interfaces/commands/revoke_permission.hpp"
#include "interfaces/commands/set_account_detail.hpp"
#include "interfaces/commands/set_quorum.hpp"
#include "interfaces/commands/subtract_asset_quantity.hpp"
#include "interfaces/commands/transfer_asset.hpp"

namespace iroha {

  using CommandError = ametsuchi::CommandError;

  /**
   * CommandResult is a return type of all execute and validate functions.
   * If execute or validate is successful, result will not contain anything
   * (void value), because execute and validate does not return any value. If
   * execution or validation is not successful, CommandResult will contain
   * Execution error with explanation
   *
   * Result is used because it allows to clear distinction between two states
   * - value and error. If we just returned error, it would be confusing, what
   * value of an error to consider as a successful state of execution or
   * validation
   */
  using CommandResult = iroha::expected::Result<void, CommandError>;


  class CommandValidator : public boost::static_visitor<CommandResult> {
   public:
    CommandValidator(std::shared_ptr<iroha::ametsuchi::WsvQuery> queries);

    template <typename CommandType>
    CommandResult operator()(const CommandType &command) {
      return hasPermissions(command, *queries, creator_account_id) |
          [&] { return isValid(command, *queries, creator_account_id); };
    }

    void setCreatorAccountId(const shared_model::interface::types::AccountIdType
                                 &creator_account_id);

   private:
    CommandResult hasPermissions(
        const shared_model::interface::AddAssetQuantity &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::AddPeer &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::AddSignatory &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::AppendRole &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::CreateAccount &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::CreateAsset &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::CreateDomain &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::CreateRole &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::DetachRole &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::GrantPermission &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::RemoveSignatory &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::RevokePermission &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::SetAccountDetail &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::SetQuorum &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::SubtractAssetQuantity &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult hasPermissions(
        const shared_model::interface::TransferAsset &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult isValid(
        const shared_model::interface::AddAssetQuantity &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult isValid(const shared_model::interface::AddPeer &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(const shared_model::interface::AddSignatory &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(const shared_model::interface::AppendRole &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(const shared_model::interface::CreateAccount &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(const shared_model::interface::CreateAsset &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(const shared_model::interface::CreateDomain &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(const shared_model::interface::CreateRole &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(const shared_model::interface::DetachRole &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(
        const shared_model::interface::GrantPermission &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult isValid(
        const shared_model::interface::RemoveSignatory &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult isValid(
        const shared_model::interface::RevokePermission &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult isValid(
        const shared_model::interface::SetAccountDetail &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult isValid(const shared_model::interface::SetQuorum &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    CommandResult isValid(
        const shared_model::interface::SubtractAssetQuantity &command,
        iroha::ametsuchi::WsvQuery &queries,
        const shared_model::interface::types::AccountIdType
            &creator_account_id);

    CommandResult isValid(const shared_model::interface::TransferAsset &command,
                          iroha::ametsuchi::WsvQuery &queries,
                          const shared_model::interface::types::AccountIdType
                              &creator_account_id);

    std::shared_ptr<iroha::ametsuchi::WsvQuery> queries;
    shared_model::interface::types::AccountIdType creator_account_id;
  };
}  // namespace iroha

#endif  // IROHA_COMMAND_EXECUTOR_HPP
