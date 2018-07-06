/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gmock/gmock.h>
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/transaction.hpp"

namespace iface = shared_model::interface;

struct BlockMock : public iface::Block {
  MOCK_CONST_METHOD0(txsNumber, iface::types::TransactionsNumberType());
  MOCK_CONST_METHOD0(transactions, iface::types::TransactionsCollectionType());
  MOCK_CONST_METHOD0(height, iface::types::HeightType());
  MOCK_CONST_METHOD0(prevHash, const iface::types::HashType &());
  MOCK_CONST_METHOD0(signatures, iface::types::SignatureRangeType());
  MOCK_CONST_METHOD0(createdTime, iface::types::TimestampType());
  MOCK_CONST_METHOD0(payload, const iface::types::BlobType &());
  MOCK_CONST_METHOD0(blob, const iface::types::BlobType &());
  MOCK_METHOD2(addSignature,
               bool(const shared_model::crypto::Signed &signed_blob,
                    const shared_model::crypto::PublicKey &public_key));
  MOCK_CONST_METHOD0(clone, BlockMock *());
};

struct TransactionMock : public iface::Transaction {
  MOCK_CONST_METHOD0(creatorAccountId, const iface::types::AccountIdType &());
  MOCK_CONST_METHOD0(quorum, iface::types::QuorumType());
  MOCK_CONST_METHOD0(commands, CommandsType());
  MOCK_CONST_METHOD0(reduced_payload, const iface::types::BlobType &());
  MOCK_CONST_METHOD0(batch_meta,
                     boost::optional<std::shared_ptr<iface::BatchMeta>>());
  MOCK_CONST_METHOD0(signatures, iface::types::SignatureRangeType());
  MOCK_CONST_METHOD0(createdTime, iface::types::TimestampType());
  MOCK_CONST_METHOD0(payload, const iface::types::BlobType &());
  MOCK_CONST_METHOD0(blob, const iface::types::BlobType &());
  MOCK_METHOD2(addSignature,
               bool(const shared_model::crypto::Signed &signed_blob,
                    const shared_model::crypto::PublicKey &public_key));
  MOCK_CONST_METHOD0(clone, TransactionMock *());
};

struct SignatureMock : public iface::Signature {
  MOCK_CONST_METHOD0(publicKey, const PublicKeyType &());
  MOCK_CONST_METHOD0(signedData, const SignedType &());
  MOCK_CONST_METHOD0(clone, SignatureMock *());
};
