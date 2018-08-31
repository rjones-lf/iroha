/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_DEFAULT_BUILDERS_HPP
#define IROHA_DEFAULT_BUILDERS_HPP

#include "builders/protobuf/query_responses/proto_block_query_response_builder.hpp"
#include "builders/protobuf/transaction_responses/proto_transaction_status_builder.hpp"
#include "builders/query_responses/block_query_response_builder.hpp"
#include "builders/transaction_responses/transaction_status_builder.hpp"
#include "validators/amount_true_validator.hpp"
#include "validators/field_validator.hpp"

namespace shared_model {
  namespace builder {

    using DefaultTransactionStatusBuilder =
        shared_model::builder::TransactionStatusBuilder<
            shared_model::proto::TransactionStatusBuilder>;

    using DefaultBlockQueryResponseBuilder =
        shared_model::builder::BlockQueryResponseBuilder<
            shared_model::proto::BlockQueryResponseBuilder>;

  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_DEFAULT_BUILDERS_HPP
