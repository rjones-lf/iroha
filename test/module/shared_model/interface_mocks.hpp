/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_INTERFACE_MOCKS_HPP
#define IROHA_SHARED_MODEL_INTERFACE_MOCKS_HPP

#include <gmock/gmock.h>
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/transaction.hpp"

struct BlockMock : public shared_model::interface::Block {
  MOCK_CONST_METHOD0(txsNumber,
                     shared_model::interface::types::TransactionsNumberType());
  MOCK_CONST_METHOD0(
      transactions,
      shared_model::interface::types::TransactionsCollectionType());
  MOCK_CONST_METHOD0(height, shared_model::interface::types::HeightType());
  MOCK_CONST_METHOD0(prevHash,
                     const shared_model::interface::types::HashType &());
  MOCK_CONST_METHOD0(signatures,
                     shared_model::interface::types::SignatureRangeType());
  MOCK_CONST_METHOD0(createdTime,
                     shared_model::interface::types::TimestampType());
  MOCK_CONST_METHOD0(payload,
                     const shared_model::interface::types::BlobType &());
  MOCK_CONST_METHOD0(blob, const shared_model::interface::types::BlobType &());
  MOCK_METHOD2(addSignature,
               bool(const shared_model::crypto::Signed &,
                    const shared_model::crypto::PublicKey &));
  MOCK_CONST_METHOD0(clone, BlockMock *());
};

struct TransactionMock : public shared_model::interface::Transaction {
  MOCK_CONST_METHOD0(creatorAccountId,
                     const shared_model::interface::types::AccountIdType &());
  MOCK_CONST_METHOD0(quorum, shared_model::interface::types::QuorumType());
  MOCK_CONST_METHOD0(commands, CommandsType());
  MOCK_CONST_METHOD0(reduced_payload,
                     const shared_model::interface::types::BlobType &());
  MOCK_CONST_METHOD0(
      batch_meta,
      boost::optional<std::shared_ptr<shared_model::interface::BatchMeta>>());
  MOCK_CONST_METHOD0(signatures,
                     shared_model::interface::types::SignatureRangeType());
  MOCK_CONST_METHOD0(createdTime,
                     shared_model::interface::types::TimestampType());
  MOCK_CONST_METHOD0(payload,
                     const shared_model::interface::types::BlobType &());
  MOCK_CONST_METHOD0(blob, const shared_model::interface::types::BlobType &());
  MOCK_METHOD2(addSignature,
               bool(const shared_model::crypto::Signed &,
                    const shared_model::crypto::PublicKey &));
  MOCK_CONST_METHOD0(clone, TransactionMock *());
};

struct SignatureMock : public shared_model::interface::Signature {
  MOCK_CONST_METHOD0(publicKey, const PublicKeyType &());
  MOCK_CONST_METHOD0(signedData, const SignedType &());
  MOCK_CONST_METHOD0(clone, SignatureMock *());
};

#endif  // IROHA_SHARED_MODEL_INTERFACE_MOCKS_HPP
