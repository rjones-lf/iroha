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

#ifndef IROHA_SHARED_MODEL_ACCOUNT_ASSET_HPP
#define IROHA_SHARED_MODEL_ACCOUNT_ASSET_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/amount.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Representation of wallet in system
     */
    class AccountAsset : public ModelPrimitive<AccountAsset> {
     public:
      /**
       * @return Identity of user, for fetching data
       */
      virtual const types::AccountIdType &accountId() const = 0;

      /**
       * @return Identity of asset, for fetching data
       */
      virtual const types::AssetIdType &assetId() const = 0;

      /**
       * @return Current balance
       */
      virtual const Amount &balance() const = 0;

      /**
       * Stringify the data.
       * @return the content of account asset.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("AccountAsset")
            .append("accountId", accountId())
            .append("assetId", assetId())
            .append("balance", balance().toString())
            .finalize();
      }

      /**
       * Checks equality of objects inside
       * @param rhs - other wrapped value
       * @return true, if wrapped objects are same
       */
      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId() and assetId() == rhs.assetId()
            and balance() == rhs.balance();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_ACCOUNT_ASSET_HPP
