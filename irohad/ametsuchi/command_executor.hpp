/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
#define IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP

#include "backend/protobuf/permissions.hpp"
#include "common/result.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Peer;
  }
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {

    /**
     * Error returned by wsv command.
     * It is a string which contains what action has failed (e.g, "failed to
     * insert role"), and an error which was provided by underlying
     * implementation (e.g, database exception info)
     */
    using WsvError = std::string;

    /**
     *  If command is successful, we assume changes are made,
     *  and do not need anything
     *  If something goes wrong, Result will contain WsvError
     *  with additional information
     */
    using WsvCommandResult = expected::Result<void, WsvError>;

    class CommandExecutor {
     public:
      virtual ~CommandExecutor() = default;

      /**
       * AddAssetQuantity sql executor
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult addAssetQuantity(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AssetIdType &asset_id,
          const std::string &amount,
          const shared_model::interface::types::PrecisionType) = 0;

      /**
       *
       * @param peer
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult addPeer(
          const shared_model::interface::Peer &peer) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult addSignatory(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::PubkeyType &signatory) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult appendRole(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::RoleIdType &role_name) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult createAccount(
          const shared_model::interface::types::AccountIdType &account_name,
          const shared_model::interface::types::DomainIdType &domain_id,
          const shared_model::interface::types::PubkeyType &pubkey) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult createAsset(
          const shared_model::interface::types::AssetIdType &asset_id,
          const shared_model::interface::types::DomainIdType &domain_id,
          const shared_model::interface::types::PrecisionType precision) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult createDomain(
          const shared_model::interface::types::DomainIdType &domain_id,
          const shared_model::interface::types::RoleIdType &default_role) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult createRole(
          const shared_model::interface::types::RoleIdType &role_id,
          const shared_model::interface::RolePermissionSet &default_role) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult detachRole(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::RoleIdType &role_name) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult grantPermission(
          const shared_model::interface::types::AccountIdType
              &permittee_account_id,
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::permissions::Grantable
              &permission) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult removeSignatory(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::PubkeyType &pubkey) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult revokePermission(
          const shared_model::interface::types::AccountIdType
              &permittee_account_id,
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::permissions::Grantable
              &permission) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult setAccountDetail(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AccountIdType
              &creator_account_id,
          const std::string &key,
          const std::string &value) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult setQuorum(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::QuorumType quorum) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult subtractAssetQuantity(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AssetIdType &asset_id,
          const std::string &amount,
          const shared_model::interface::types::PrecisionType) = 0;

      /**
       *
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult transferAsset(
          const shared_model::interface::types::AccountIdType &src_account_id,
          const shared_model::interface::types::AccountIdType &dest_account_id,
          const shared_model::interface::types::AssetIdType &asset_id,
          const std::string &amount,
          const shared_model::interface::types::PrecisionType precision) = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
