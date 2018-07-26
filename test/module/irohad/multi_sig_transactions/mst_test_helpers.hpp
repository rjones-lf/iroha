/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MST_TEST_HELPERS_HPP
#define IROHA_MST_TEST_HELPERS_HPP

#include <string>
#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/batch_helper.hpp"
#include "interfaces/common_objects/types.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "multi_sig_transactions/mst_types.hpp"

inline auto makeKey() {
  return shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
}

inline auto txBuilder(
    const shared_model::interface::types::CounterType &counter,
    iroha::TimeType created_time = iroha::time::now(),
    uint8_t quorum = 3) {
  return TestTransactionBuilder()
      .createdTime(created_time)
      .creatorAccountId("user@test")
      .setAccountQuorum("user@test", counter)
      .quorum(quorum);
}

template <typename... TxBuilders>
auto makeTestBatch(TxBuilders... builders) {
  return framework::batch::makeTestBatch(builders...);
}

inline auto makeTx(const shared_model::interface::types::CounterType &counter,
                   iroha::TimeType created_time = iroha::time::now(),
                   shared_model::crypto::Keypair keypair = makeKey(),
                   uint8_t quorum = 3) {
  return std::make_shared<shared_model::proto::Transaction>(
      shared_model::proto::TransactionBuilder()
          .createdTime(created_time)
          .creatorAccountId("user@test")
          .setAccountQuorum("user@test", counter)
          .quorum(quorum)
          .build()
          .signAndAddSignature(keypair)
          .finish());
}

inline auto makePeer(const std::string &address, const std::string &pub_key) {
  return std::make_shared<shared_model::proto::Peer>(
      shared_model::proto::PeerBuilder()
          .address(address)
          .pubkey(shared_model::crypto::PublicKey(pub_key))
          .build());
}

#endif  // IROHA_MST_TEST_HELPERS_HPP
