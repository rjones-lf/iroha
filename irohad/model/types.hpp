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

#ifndef IROHA_MODEL_TYPES_HPP
#define IROHA_MODEL_TYPES_HPP

#include <memory>
#include "model/peer.hpp"
#include "model/transaction.hpp"
#include "model/transaction_response.hpp"

namespace iroha {
  using TxType = model::Transaction;
  using SharedTx = std::shared_ptr<TxType>;
  using ConstPeer = const model::Peer;
  using TimeType = model::Transaction::TimeType;
  using TxResponse = std::shared_ptr<model::TransactionResponse>;

  template <typename T>
  using ConstRefT = const T &;

  using ConstRefTransaction = ConstRefT<SharedTx>;
  using ConstRefPeer = ConstRefT<model::Peer>;
  using ConstRefTime = ConstRefT<TimeType>;
}  // namespace iroha
#endif  // IROHA_MODEL_TYPES_HPP
