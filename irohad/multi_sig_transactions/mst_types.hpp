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

#ifndef IROHA_MST_TYPES_HPP
#define IROHA_MST_TYPES_HPP

#include <memory>
#include "interfaces/common_objects/peer.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction_responses/tx_response.hpp"

namespace iroha {
  using BatchPtr = std::shared_ptr<shared_model::interface::TransactionBatch>;
  using ConstPeer = const shared_model::interface::Peer;
  using TimeType = shared_model::interface::types::TimestampType;
  using TxResponse =
      std::shared_ptr<shared_model::interface::TransactionResponse>;

  template <typename T>
  using ConstRefT = const T &;

  using ConstRefBatch = ConstRefT<BatchPtr>;
  using ConstRefPeer = ConstRefT<shared_model::interface::Peer>;
  using ConstRefTime = ConstRefT<TimeType>;

  class MstState;

  using ConstRefState = ConstRefT<MstState>;

  using DataType = BatchPtr;
}  // namespace iroha
#endif  // IROHA_MST_TYPES_HPP
