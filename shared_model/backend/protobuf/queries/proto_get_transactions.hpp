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

#ifndef IROHA_PROTO_GET_TRANSACTIONS_HPP
#define IROHA_PROTO_GET_TRANSACTIONS_HPP

#include "interfaces/queries/get_transactions.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetTransactions final
        : public CopyableProto<interface::GetTransactions,
                               iroha::protocol::Query,
                               GetTransactions> {
     public:
      template <typename QueryType>
      explicit GetTransactions(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)) {}

      GetTransactions(const GetTransactions &o) : GetTransactions(o.proto_) {}

      GetTransactions(GetTransactions &&o) noexcept
          : GetTransactions(std::move(o.proto_)) {}

      const TransactionHashesType &transactionHashes() const override {
        return *transaction_hashes_;
      }

     private:
      // ------------------------------| fields |-------------------------------

      const iroha::protocol::GetTransactions &get_transactions_{
          proto_->payload().get_transactions()};

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<TransactionHashesType> transaction_hashes_{[this] {
        return boost::accumulate(get_transactions_.tx_hashes(),
                                 TransactionHashesType{},
                                 [](auto &&acc, const auto &hash) {
                                   acc.emplace_back(hash);
                                   return std::forward<decltype(acc)>(acc);
                                 });
      }};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_TRANSACTIONS_HPP
