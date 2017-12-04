/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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
#ifndef IROHA_SUBTRACT_ASSET_QUANTITY_HPP
#define IROHA_SUBTRACT_ASSET_QUANTITY_HPP


#include <model/command.hpp>
#include <string>
#include "common/types.hpp"
#include "amount/amount.hpp"

namespace iroha {
    namespace model {

        /**
         * Subtract amount of asset to an account
         */
        struct SubtractAssetQuantity : public Command {
            /**
             * Account where to subtract assets
             */
            std::string account_id{};

            /**
             * Asset to issue
             * Note: must exist in the system
             */
            std::string asset_id{};

            /**
             * Amount to add to account asset
             */
            Amount amount{};

            bool operator==(const Command &command) const override;

            SubtractAssetQuantity() {}

            SubtractAssetQuantity(const std::string &account_id,
                             const std::string &asset_id, Amount amount)
              : account_id(account_id), asset_id(asset_id), amount(amount) {}
        };
    }  // namespace model
}  // namespace iroha
#endif //IROHA_SUBTRACT_ASSET_QUANTITY_HPP
