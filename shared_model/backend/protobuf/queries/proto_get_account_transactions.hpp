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

#ifndef IROHA_GET_ACCOUNT_TRANSACTIONS_H
#define IROHA_GET_ACCOUNT_TRANSACTIONS_H

#include "interfaces/queries/get_account_transactions.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetAccountTransactions final
        : public CopyableProto<interface::GetAccountTransactions,
                               iroha::protocol::Query,
                               GetAccountTransactions> {
     public:
      template <typename QueryType>
      explicit GetAccountTransactions(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            account_transactions_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::get_account_transactions)) {}

      GetAccountTransactions(const GetAccountTransactions &o)
          : GetAccountTransactions(o.proto_) {}

      GetAccountTransactions(GetAccountTransactions &&o) noexcept
          : GetAccountTransactions(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return account_transactions_->account_id();
      }

     private:
      // ------------------------------| fields |-------------------------------

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::GetAccountTransactions &>
          account_transactions_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_GET_ACCOUNT_TRANSACTIONS_H
