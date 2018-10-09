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

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "cryptography/hash.hpp"
#include "queries.pb.h"

namespace shared_model {
  namespace proto {
    class GetTransactions final
        : public CopyableProto<interface::GetTransactions,
                               iroha::protocol::Query,
                               GetTransactions> {
     public:
      template <typename QueryType>
      explicit GetTransactions(QueryType &&query);

      GetTransactions(const GetTransactions &o);

      GetTransactions(GetTransactions &&o) noexcept;

      const TransactionHashesType &transactionHashes() const override;

     private:
      // ------------------------------| fields |-------------------------------

      const iroha::protocol::GetTransactions &get_transactions_;

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<TransactionHashesType> transaction_hashes_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_TRANSACTIONS_HPP
