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

#ifndef IROHA_SHARED_MODEL_PROTO_TRANSACTION_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_TRANSACTION_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/transaction.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/query_responses/transactions_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class TransactionsResponse final
        : public CopyableProto<interface::TransactionsResponse,
                               iroha::protocol::QueryResponse,
                               TransactionsResponse> {
     public:
      template <typename QueryResponseType>
      explicit TransactionsResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      TransactionsResponse(const TransactionsResponse &o)
          : TransactionsResponse(o.proto_) {}

      TransactionsResponse(TransactionsResponse &&o)
          : TransactionsResponse(std::move(o.proto_)) {}

      interface::types::TransactionsCollectionType transactions()
          const override {
        return *transactions_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::TransactionsResponse &transactionResponse_{
          proto_->transactions_response()};

      const Lazy<interface::types::TransactionsCollectionType> transactions_{
          [this] {
            return boost::accumulate(
                transactionResponse_.transactions(),
                interface::types::TransactionsCollectionType{},
                [](auto &&txs, const auto &tx) {
                  txs.emplace_back(new Transaction(tx));
                  return std::move(txs);
                });
          }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_TRANSACTION_RESPONSE_HPP
