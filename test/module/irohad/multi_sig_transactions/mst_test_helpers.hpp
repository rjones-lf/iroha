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

#ifndef IROHA_MST_TEST_HELPERS_HPP
#define IROHA_MST_TEST_HELPERS_HPP

#include <string>
#include "common/types.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "model/converters/pb_transaction_factory.hpp"
#include "model/transaction.hpp"
#include "multi_sig_transactions/mst_types.hpp"

using namespace std;
using namespace iroha;
using namespace iroha::model;

inline auto generateSignature(const string &sign_value) {
  // TODO move out this code after reworking model builders
  Signature s;
  s.signature = stringToBytesFiller<Signature::SignatureType>(sign_value);
  s.pubkey = stringToBytesFiller<Signature::KeyType>(sign_value);
  return s;
}

inline auto makeTx(const string &hash_value,
                   const string &signature_value,
                   uint8_t quorum = 3,
                   iroha::TimeType created_time = 1) {
  Transaction tx{};
  tx.tx_hash = stringToBytesFiller<Transaction::HashType>(hash_value);
  tx.signatures = {generateSignature(signature_value)};
  tx.quorum = quorum;
  tx.created_ts = created_time;
  return make_shared<Transaction>(tx);
}

inline auto makeTxWithCorrectHash(const string &signature_value,
                                  uint8_t quorum = 3,
                                  iroha::TimeType created_time = 1) {
  auto tx = makeTx("0", signature_value, quorum, created_time);
  tx->tx_hash = iroha::sha3_256(iroha::model::converters::PbTransactionFactory()
                                    .serialize(*tx)
                                    .payload()
                                    .SerializeAsString());
  return tx;
}

inline auto makePeer(const string &address, const string &pub_key) {
  iroha::model::Peer p;
  p.address = address;
  p.pubkey = stringToBytesFiller<iroha::model::Peer::KeyType>(pub_key);
  return p;
}

#endif  // IROHA_MST_TEST_HELPERS_HPP
