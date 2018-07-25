/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_POSTGRES_COMMAND_EXECUTOR_HPP
#define IROHA_POSTGRES_COMMAND_EXECUTOR_HPP

#include "ametsuchi/command_executor.hpp"
#include "ametsuchi/impl/soci_utils.hpp"

namespace iroha {
  namespace ametsuchi {

    class PostgresCommandExecutor : public CommandExecutor {
     public:
      explicit PostgresCommandExecutor(soci::session &transaction);

      void setCreatorAccountId(
          const shared_model::interface::types::AccountIdType
              &creator_account_id) override;

      CommandResult operator()(
          const shared_model::interface::AddAssetQuantity &command) override;

      CommandResult operator()(
          const shared_model::interface::AddPeer &command) override;

      CommandResult operator()(
          const shared_model::interface::AddSignatory &command) override;

      CommandResult operator()(
          const shared_model::interface::AppendRole &command) override;

      CommandResult operator()(
          const shared_model::interface::CreateAccount &command) override;

      CommandResult operator()(
          const shared_model::interface::CreateAsset &command) override;

      CommandResult operator()(
          const shared_model::interface::CreateDomain &command) override;

      CommandResult operator()(
          const shared_model::interface::CreateRole &command) override;

      CommandResult operator()(
          const shared_model::interface::DetachRole &command) override;

      CommandResult operator()(
          const shared_model::interface::GrantPermission &command) override;

      CommandResult operator()(
          const shared_model::interface::RemoveSignatory &command) override;

      CommandResult operator()(
          const shared_model::interface::RevokePermission &command) override;

      CommandResult operator()(
          const shared_model::interface::SetAccountDetail &command) override;

      CommandResult operator()(
          const shared_model::interface::SetQuorum &command) override;

      CommandResult operator()(
          const shared_model::interface::SubtractAssetQuantity &command)
          override;

      CommandResult operator()(
          const shared_model::interface::TransferAsset &command) override;

     private:
      soci::session &sql_;

      shared_model::interface::types::AccountIdType creator_account_id_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_COMMAND_EXECUTOR_HPP
