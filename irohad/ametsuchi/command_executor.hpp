/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
#define IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP

#include "common/result.hpp"
#include "interfaces/common_objects/types.hpp"

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
          const int precision) = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_COMMAND_EXECUTOR_HPP
