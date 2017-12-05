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

#include "interfaces/common_objects/types.hpp"
#include "interfaces/primitive.hpp"
#include "model/queries/get_transactions.hpp"

#ifndef IROHA_SHARED_MODEL_GET_TRANSACTIONS_HPP
#define IROHA_SHARED_MODEL_GET_TRANSACTIONS_HPP

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
       * @return assetId of requested transactions
       */
      virtual const types::AccountIdType &assetId() const = 0;

      OldModelType *makeOldModel() const override {
        auto oldModel = new iroha::model::GetAccountAssetTransactions;
        oldModel->account_id = accountId();
        oldModel->asset_id = assetId();
        return oldModel;
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("GetAccountAssetTransactions")
            .append("account_id", accountId())
            .append("asset_id", assetId())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId() and assetId() == rhs.assetId();
      }
    };

    /**
     * Query for getting transactions of account
     */
    class GetAccountTransactions
        : public Primitive<GetAccountTransactions,
                           iroha::model::GetAccountTransactions> {
     public:
      /**
       * @return account_id of requested transactions
       */
      virtual const types::AccountIdType &accountId() const = 0;

      virtual OldModelType *makeOldModel() const override {
        auto oldModel = new iroha::model::GetAccountTransactions;
        oldModel->account_id = accountId();
        return oldModel;
      }

      virtual std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("GetAccountTransactions")
            .append("account_id", accountId())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_GET_TRANSACTIONS_HPP
