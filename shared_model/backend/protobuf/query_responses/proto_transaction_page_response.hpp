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

#ifndef IROHA_SHARED_MODEL_PROTO_TRANSACTION_PAGE_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_TRANSACTION_PAGE_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/transaction.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/query_responses/transactions_page_response.hpp"
#include "qry_responses.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class TransactionsPageResponse final
        : public CopyableProto<interface::TransactionsPageResponse,
                               iroha::protocol::QueryResponse,
                               TransactionsPageResponse> {
     public:
      template <typename QueryResponseType>
      explicit TransactionsPageResponse(QueryResponseType &&queryResponse);

      TransactionsPageResponse(const TransactionsPageResponse &o);

      TransactionsPageResponse(TransactionsPageResponse &&o);

      interface::types::TransactionsCollectionType transactions()
          const override;

      interface::types::HashType nextTxHash() const override;

      interface::types::TransactionsNumberType allTransactionsSize()
          const override;

     private:
      const iroha::protocol::TransactionsPageResponse &transactionPageResponse_;
      const std::vector<proto::Transaction> transactions_;
      interface::types::HashType next_hash_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_TRANSACTION_PAGE_RESPONSE_HPP
