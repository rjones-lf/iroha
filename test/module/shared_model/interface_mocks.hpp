/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_INTERFACE_MOCKS_HPP
#define IROHA_INTERFACE_MOCKS_HPP

#include <gmock/gmock.h>
#include "interfaces/common_objects/peer.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
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
               bool(const shared_model::crypto::Signed &,
                    const shared_model::crypto::PublicKey &));
  MOCK_CONST_METHOD0(clone, BlockMock *());
};

struct MockTransaction : public iface::Transaction {
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
               bool(const shared_model::crypto::Signed &,
                    const shared_model::crypto::PublicKey &));
  MOCK_CONST_METHOD0(clone, MockTransaction *());
  MOCK_CONST_METHOD0(reducedPayload, const iface::types::BlobType &());
  MOCK_CONST_METHOD0(batchMeta,
                     boost::optional<std::shared_ptr<iface::BatchMeta>>());
};

struct SignatureMock : public iface::Signature {
  MOCK_CONST_METHOD0(publicKey, const PublicKeyType &());
  MOCK_CONST_METHOD0(signedData, const SignedType &());
  MOCK_CONST_METHOD0(clone, SignatureMock *());
};

struct MockProposal : public iface::Proposal {
  MOCK_CONST_METHOD0(transactions, iface::types::TransactionsCollectionType());
  MOCK_CONST_METHOD0(height, iface::types::HeightType());
  MOCK_CONST_METHOD0(createdTime, iface::types::TimestampType());
  MOCK_CONST_METHOD0(blob, const iface::types::BlobType &());
  MOCK_CONST_METHOD0(clone, MockProposal *());
};

struct MockPeer : public iface::Peer {
  MOCK_CONST_METHOD0(address, const iface::types::AddressType &());
  MOCK_CONST_METHOD0(pubkey, const iface::types::PubkeyType &());
  MOCK_CONST_METHOD0(clone, MockPeer *());
};

struct MockUnsafeProposalFactory : public iface::UnsafeProposalFactory {
  MOCK_METHOD3(unsafeCreateProposal,
               std::unique_ptr<iface::Proposal>(
                   iface::types::HeightType,
                   iface::types::TimestampType,
                   const iface::types::TransactionsCollectionType &));
};

#endif  // IROHA_INTERFACE_MOCKS_HPP
