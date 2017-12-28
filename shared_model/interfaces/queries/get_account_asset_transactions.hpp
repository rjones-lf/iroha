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

#include <boost/algorithm/string/join.hpp>
#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "model/queries/get_account_asset_transactions.hpp"

#ifndef IROHA_SHARED_MODEL_GET_ACCOUNT_ASSET_TRANSACTIONS_HPP
#define IROHA_SHARED_MODEL_GET_ACCOUNT_ASSET_TRANSACTIONS_HPP

namespace shared_model {
  namespace interface {

    /**
     * Query for getting transactions of given asset of an account
     */
    class GetAccountAssetTransactions
        : public Primitive<GetAccountAssetTransactions,
                           iroha::model::GetAccountAssetTransactions> {
     public:
      /**
       * @return account_id of requested transactions
       */
      virtual const types::AccountIdType &accountId() const = 0;
      /**
       * @return assetsId of requested transactions
       */
      virtual const types::AssetIdCollectionType &assetsId() const = 0;
      //virtual const

      OldModelType *makeOldModel() const override {
        auto oldModel = new iroha::model::GetAccountAssetTransactions;
        oldModel->account_id = accountId();
        oldModel->assets_id = assetsId();
        return oldModel;
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("GetAccountAssetTransactions")
            .append("account_id", accountId())
            .append("assets_id", boost::algorithm::join(assetsId(), ","))
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId() and assetsId() == rhs.assetsId();
      }
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_GET_ACCOUNT_ASSET_TRANSACTIONS_HPP
