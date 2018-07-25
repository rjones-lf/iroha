/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "module/irohad/pending_txs_storage/pending_txs_storage_fixture.hpp"

#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "datetime/time.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"

PendingTxsStorageFixture::TxData::TxData(const types::AccountIdType &creator,
                                         const types::QuorumType &quorum,
                                         const unsigned &keys)
    : creator_account_id_(creator),
      transaction_quorum_(quorum),
      created_time_(iroha::time::now()) {
  addKeys(keys);
}

void PendingTxsStorageFixture::TxData::addKeys(const unsigned &amount) {
  for (auto i = 0u; i < amount; ++i) {
    keys_.push_back(crypto::DefaultCryptoAlgorithmType::generateKeypair());
  }
}

PendingTxsStorageFixture::PendingTxsStorageFixture()
    : log_(logger::log("PendingTxsStorageTest")){};

std::shared_ptr<PendingTxsStorageFixture::Batch>
PendingTxsStorageFixture::generateSharedBatch(
    const std::vector<PendingTxsStorageFixture::TxData> &source) const {
  std::vector<types::HashType> hashes;
  std::vector<TestTransactionBuilder> builders;
  std::vector<proto::Transaction> transactions;

  for (const auto &tx : source) {
    auto reduced_tx =
        TestTransactionBuilder()
            .createdTime(tx.created_time_)
            .creatorAccountId(tx.creator_account_id_)
            .quorum(tx.transaction_quorum_)
            .setAccountQuorum(tx.creator_account_id_, tx.transaction_quorum_);
    builders.push_back(reduced_tx);
    hashes.push_back(reduced_tx.build().reducedHash());
  }

  if (source.size() != builders.size()) {
    log_->error("Unable to prepare transactions batch");
    return {};
  }

  for (auto i = 0u; i < source.size(); ++i) {
    auto transaction =
        builders[i].batchMeta(types::BatchType::ATOMIC, hashes).build();
    for (const auto &key : source[i].keys_) {
      auto signed_blob = ::crypto::CryptoSigner<>::sign(
          shared_model::crypto::Blob(transaction.payload()), key);
      transaction.addSignature(signed_blob, key.publicKey());
    }
    transactions.push_back(transaction);
  }

  types::SharedTxsCollectionType interface_txs;
  for (auto &tx : transactions) {
    std::shared_ptr<shared_model::interface::Transaction> shared_tx =
        std::make_shared<shared_model::proto::Transaction>(std::move(tx));
    interface_txs.push_back(shared_tx);
  }
  auto batch_res =
      shared_model::interface::TransactionBatch::createTransactionBatch(
          interface_txs, TxsValidator());

  return batch_res.match(
      [&](iroha::expected::Value<Batch> &batch) {
        std::shared_ptr<Batch> shared_batch =
            std::make_shared<Batch>(std::move(batch.value));
        return shared_batch;
      },
      [&](iroha::expected::Error<std::string> &)
          -> std::shared_ptr<PendingTxsStorageFixture::Batch> {
        return nullptr;
      });
}
