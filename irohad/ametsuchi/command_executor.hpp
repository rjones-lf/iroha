/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
#define IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP

#include <boost/format.hpp>

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
     * Error for command execution or validation
     * Contains command name, as well as an error message
     */
    struct CommandError {
      std::string command_name;
      std::string error_message;

      std::string toString() const {
        return (boost::format("%s: %s") % command_name % error_message).str();
      }
    };

    /**
     *  If command is successful, we assume changes are made,
     *  and do not need anything
     *  If something goes wrong, Result will contain Error
     *  with additional information
     */
    using CommandResult = expected::Result<void, CommandError>;

    class CommandExecutor : public boost::static_visitor<CommandResult> {
     public:
      virtual ~CommandExecutor() = default;

      virtual void setCreatorAccountId(
          const shared_model::interface::types::AccountIdType
              &creator_account_id) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::AddAssetQuantity &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::AddPeer &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::AddSignatory &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::AppendRole &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::CreateAccount &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::CreateAsset &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::CreateDomain &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::CreateRole &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::DetachRole &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::GrantPermission &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::RemoveSignatory &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::RevokePermission &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::SetAccountDetail &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::SetQuorum &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::SubtractAssetQuantity &command) = 0;

      virtual CommandResult operator()(
          const shared_model::interface::TransferAsset &command) = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
