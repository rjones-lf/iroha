/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PENDING_TXS_STORAGE_FIXTURE_HPP
#define IROHA_PENDING_TXS_STORAGE_FIXTURE_HPP

#include <gtest/gtest.h>
#include <logger/logger.hpp>
#include "backend/protobuf/transaction.hpp"
#include "cryptography/keypair.hpp"
#include "interfaces/common_objects/types.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

namespace types = shared_model::interface::types;
namespace crypto = shared_model::crypto;
namespace proto = shared_model::proto;
namespace validation = shared_model::validation;

class PendingTxsStorageFixture : public ::testing::Test {
 public:
  using TxValidator = validation::TransactionValidator<
      validation::FieldValidator,
      validation::CommandValidatorVisitor<validation::FieldValidator>>;
  using TxsValidator = validation::UnsignedTransactionsCollectionValidator<
      TxValidator,
      validation::BatchOrderValidator>;
  using Batch = shared_model::interface::TransactionBatch;

  struct TxData {
    const types::AccountIdType creator_account_id_;
    const types::QuorumType transaction_quorum_;
    const types::TimestampType created_time_;
    std::vector<crypto::Keypair> keys_;

    TxData(const types::AccountIdType &creator,
           const types::QuorumType &quorum,
           const unsigned &keys = 0);
    void addKeys(const unsigned &amount);
  };

  PendingTxsStorageFixture();

 protected:
  std::shared_ptr<PendingTxsStorageFixture::Batch> generateSharedBatch(
      const std::vector<PendingTxsStorageFixture::TxData> &source) const;

  std::shared_ptr<spdlog::logger> log_;
};

#endif  // IROHA_PENDING_TXS_STORAGE_FIXTURE_HPP
