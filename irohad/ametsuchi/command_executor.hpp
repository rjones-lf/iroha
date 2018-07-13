/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
#define IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP

#include "backend/protobuf/permissions.hpp"
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
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Peer;
  }
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {

    /**
     * Error returned by command.
     * It is a string which contains what action has failed (e.g, "failed to
     * insert role"), and an error which was provided by underlying
     * implementation (e.g, database exception info)
     */
    using Error = std::string;

    /**
     *  If command is successful, we assume changes are made,
     *  and do not need anything
     *  If something goes wrong, Result will contain Error
     *  with additional information
     */
    using CommandResult = expected::Result<void, Error>;

    class CommandExecutor : public boost::static_visitor<CommandResult> {
     public:
      virtual ~CommandExecutor() = default;

      virtual void setCreatorAccountId(
          const shared_model::interface::types::AccountIdType
              &creator_account_id) = 0;

      /**
       * AddAssetQuantity sql executor
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::AddAssetQuantity &command) = 0;

      /**
       *
       * @param peer
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::AddPeer &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::AddSignatory &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::AppendRole &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::CreateAccount &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::CreateAsset &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::CreateDomain &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::CreateRole &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::DetachRole &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::GrantPermission &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::RemoveSignatory &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::RevokePermission &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::SetAccountDetail &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::SetQuorum &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::SubtractAssetQuantity &command) = 0;

      /**
       *
       * @return CommandResult, which will contain error in case of failure
       */
      virtual CommandResult operator()(
          const shared_model::interface::TransferAsset &command) = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
