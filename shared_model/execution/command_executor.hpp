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

#ifndef IROHA_SHARED_MODEL_COMMAND_EXECUTOR_HPP
#define IROHA_SHARED_MODEL_COMMAND_EXECUTOR_HPP

#include <boost/format.hpp>
#include <boost/variant/static_visitor.hpp>

#include "ametsuchi/wsv_command.hpp"
#include "ametsuchi/wsv_query.hpp"

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

#include "builders/protobuf/common_objects/proto_amount_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_account_builder.hpp"
#include "builders/protobuf/common_objects/proto_asset_builder.hpp"
#include "builders/protobuf/common_objects/proto_domain_builder.hpp"

#include "common/result.hpp"

namespace shared_model {

  /**
   * Error for command execution.
   * Contains command name, as well as an error message
   */
  struct ExecutionError {
    std::string command_name;
    std::string error_message;

    std::string toString() const {
      return (boost::format("%s: %s") % command_name % error_message).str();
    }
  };

  /**
   * ExecutionResult is a return type of all execute functions.
   * If execute is successful, result will not contain anything (void value),
   * because execute does not return any value.
   * If execution is not successful, ExecutionResult will contain Execution
   * error with explanation
   *
   * Result is used because it allows for clear distinction between two states
   * - value and error. If we just returned error, it would be confusing, what
   * value of an error to consider as a successful state of execution
   */
  using ExecutionResult = iroha::expected::Result<void, ExecutionError>;

  class CommandExecutor : public boost::static_visitor<ExecutionResult> {
   public:
    CommandExecutor(std::shared_ptr<iroha::ametsuchi::WsvQuery> queries,
                    std::shared_ptr<iroha::ametsuchi::WsvCommand> commands);


    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::AddAssetQuantity> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::AddPeer> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::AddSignatory> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::AppendRole> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::CreateAccount> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::CreateAsset> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::CreateDomain> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::CreateRole> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::DetachRole> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::GrantPermission> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::RemoveSignatory> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::RevokePermission> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::SetAccountDetail> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::SetQuorum> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::SubtractAssetQuantity> &command);

    ExecutionResult operator()(const detail::PolymorphicWrapper<interface::TransferAsset> &command);

    void setCreatorAccountId(std::string creator_account_id);

  private:
      std::shared_ptr<iroha::ametsuchi::WsvQuery> queries;
      std::shared_ptr<iroha::ametsuchi::WsvCommand> commands;
      std::string creator_account_id;

      shared_model::proto::AmountBuilder amount_builder;
      shared_model::proto::AccountAssetBuilder account_asset_builder;
      shared_model::proto::AccountBuilder account_builder;
      shared_model::proto::AssetBuilder asset_builder;
      shared_model::proto::DomainBuilder domain_builder;
  };

  class CommandValidator : public boost::static_visitor<bool> {
   public:
    CommandValidator(std::shared_ptr<iroha::ametsuchi::WsvQuery> queries);

    template <typename CommandType>
    bool operator()(const detail::PolymorphicWrapper<CommandType> &command) {
        bool a = hasPermissions(*command.operator->(), *queries, creator_account_id);
        bool b = isValid(*command.operator->(), *queries, creator_account_id);
        return a
               and b;
    }

      void setCreatorAccountId(std::string creator_account_id);

   private:
    bool hasPermissions(const interface::AddAssetQuantity &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::AddPeer &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::AddSignatory &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::AppendRole &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::CreateAccount &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::CreateAsset &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::CreateDomain &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::CreateRole &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::DetachRole &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::GrantPermission &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::RemoveSignatory &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::RevokePermission &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::SetAccountDetail &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::SetQuorum &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::SubtractAssetQuantity &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool hasPermissions(const interface::TransferAsset &command,
                        iroha::ametsuchi::WsvQuery &queries,
                        const std::string &creator_account_id);

    bool isValid(const interface::AddAssetQuantity &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::AddPeer &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::AddSignatory &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::AppendRole &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::CreateAccount &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::CreateAsset &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::CreateDomain &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::CreateRole &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::DetachRole &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::GrantPermission &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::RemoveSignatory &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::RevokePermission &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::SetAccountDetail &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::SetQuorum &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::SubtractAssetQuantity &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);

    bool isValid(const interface::TransferAsset &command,
                 iroha::ametsuchi::WsvQuery &queries,
                 const std::string &creator_account_id);



      std::shared_ptr<iroha::ametsuchi::WsvQuery> queries;
      std::string creator_account_id;
  };
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_COMMAND_EXECUTOR_HPP
