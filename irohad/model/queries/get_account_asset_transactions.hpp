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

#ifndef IROHA_GET_ACCOUNT_ASSET_TRANSACTIONS_HPP
#define IROHA_GET_ACCOUNT_ASSET_TRANSACTIONS_HPP

#include <model/query.hpp>
#include <string>
#include <vector>
#include "model/queries/pager.hpp"

namespace iroha {
  namespace model {
    /**
     * Query for getting transactions of given asset of an account
     */
    struct GetAccountAssetTransactions : Query {
      /**
       * Account identifier
       */
      std::string account_id{};

      /**
       * Asset identifiers
       */
      std::vector<std::string> assets_id{};

      /**
       * Pager for transactions
       */
      Pager pager{};

      using AssetsIdType = decltype(assets_id);

      bool operator==(GetAccountAssetTransactions const& rhs) const {
        return account_id == rhs.account_id and assets_id == rhs.assets_id
               and pager == rhs.pager;
      }
      bool operator!=(GetAccountAssetTransactions const& rhs) const {
        return not(operator==(rhs));
      }
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_GET_ACCOUNT_ASSET_TRANSACTIONS_HPP
