/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_INTERFACE_MOCKS_HPP
#define IROHA_SHARED_MODEL_INTERFACE_MOCKS_HPP

#include <gmock/gmock.h>
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/public_key.hpp"
#include "cryptography/signed.hpp"
#include "interfaces/commands/command.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "interfaces/transaction.hpp"

struct MockBlock : public shared_model::interface::Block {
  MOCK_CONST_METHOD0(txsNumber,
                     shared_model::interface::types::TransactionsNumberType());
  MOCK_CONST_METHOD0(
      transactions,
      shared_model::interface::types::TransactionsCollectionType());
  MOCK_CONST_METHOD0(rejected_transactions_hashes,
                     shared_model::interface::types::HashCollectionType());
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
  MOCK_CONST_METHOD0(clone, MockBlock *());
};

struct MockTransaction : public shared_model::interface::Transaction {
  MOCK_CONST_METHOD0(creatorAccountId,
                     const shared_model::interface::types::AccountIdType &());
  MOCK_CONST_METHOD0(quorum, shared_model::interface::types::QuorumType());
  MOCK_CONST_METHOD0(commands, CommandsType());
  MOCK_CONST_METHOD0(reducedHash,
                     const shared_model::interface::types::HashType &());
  MOCK_CONST_METHOD0(hash, const shared_model::interface::types::HashType &());
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
  MOCK_CONST_METHOD0(clone, MockTransaction *());
  MOCK_CONST_METHOD0(reducedPayload,
                     const shared_model::interface::types::BlobType &());
  MOCK_CONST_METHOD0(
      batchMeta,
      boost::optional<std::shared_ptr<shared_model::interface::BatchMeta>>());
};

/**
 * Creates mock transaction with provided hash
 * @param hash -- const ref to hash to be returned by the transaction
 * @return shared_ptr for transaction
 */
auto createMockTransactionWithHash(
    const shared_model::interface::types::HashType &hash) {
  using ::testing::NiceMock;
  using ::testing::ReturnRefOfCopy;

  auto res = std::make_shared<NiceMock<MockTransaction>>();

  ON_CALL(*res, hash()).WillByDefault(ReturnRefOfCopy(hash));

  return res;
}

struct MockTransactionBatch : public shared_model::interface::TransactionBatch {
  MOCK_CONST_METHOD0(
      transactions,
      const shared_model::interface::types::SharedTxsCollectionType &());
  MOCK_CONST_METHOD0(reducedHash,
                     const shared_model::interface::types::HashType &());
  MOCK_CONST_METHOD0(hasAllSignatures, bool());
  MOCK_METHOD3(addSignature,
               bool(size_t,
                    const shared_model::crypto::Signed &,
                    const shared_model::crypto::PublicKey &));
  MOCK_CONST_METHOD1(Equals,
                     bool(const shared_model::interface::TransactionBatch &));
  virtual bool operator==(
      const shared_model::interface::TransactionBatch &rhs) const override {
    return Equals(rhs);
  }
  MOCK_CONST_METHOD1(NotEquals,
                     bool(const shared_model::interface::TransactionBatch &));
  MOCK_CONST_METHOD0(clone, MockTransactionBatch *());
};

/**
 * Creates mock batch with provided hash
 * @param hash -- const ref to reduced hash to be returned by the batch
 * @return shared_ptr for batch
 */
auto createMockBatchWithHash(
    const shared_model::interface::types::HashType &hash) {
  using ::testing::NiceMock;
  using ::testing::ReturnRefOfCopy;

  auto res = std::make_shared<NiceMock<MockTransactionBatch>>();

  ON_CALL(*res, reducedHash()).WillByDefault(ReturnRefOfCopy(hash));

  return res;
}

/**
 * Creates mock batch with provided transactions
 * @param txs -- const ref to hash to be returned by the batch
 * @return shared_ptr for batch
 */
auto createMockBatchWithTransactions(
    const shared_model::interface::types::SharedTxsCollectionType &txs) {
  using ::testing::NiceMock;
  using ::testing::ReturnRefOfCopy;

  auto res = std::make_shared<NiceMock<MockTransactionBatch>>();

  ON_CALL(*res, transactions()).WillByDefault(ReturnRefOfCopy(txs));

  ON_CALL(*res, reducedHash())
      .WillByDefault(ReturnRefOfCopy(shared_model::crypto::Hash::fromHexString(
          shared_model::crypto::DefaultCryptoAlgorithmType::generateSeed()
              .hex())));

  return res;
}

struct MockSignature : public shared_model::interface::Signature {
  MOCK_CONST_METHOD0(publicKey, const PublicKeyType &());
  MOCK_CONST_METHOD0(signedData, const SignedType &());
  MOCK_CONST_METHOD0(clone, MockSignature *());
};

struct MockProposal : public shared_model::interface::Proposal {
  MOCK_CONST_METHOD0(
      transactions,
      shared_model::interface::types::TransactionsCollectionType());
  MOCK_CONST_METHOD0(height, shared_model::interface::types::HeightType());
  MOCK_CONST_METHOD0(createdTime,
                     shared_model::interface::types::TimestampType());
  MOCK_CONST_METHOD0(blob, const shared_model::interface::types::BlobType &());
  MOCK_CONST_METHOD0(hash, const shared_model::interface::types::HashType &());
  MOCK_CONST_METHOD0(clone, MockProposal *());
};

struct MockPeer : public shared_model::interface::Peer {
  MOCK_CONST_METHOD0(address,
                     const shared_model::interface::types::AddressType &());
  MOCK_CONST_METHOD0(pubkey,
                     const shared_model::interface::types::PubkeyType &());
  MOCK_CONST_METHOD0(clone, MockPeer *());
};

struct MockUnsafeProposalFactory
    : public shared_model::interface::UnsafeProposalFactory {
  MOCK_METHOD3(unsafeCreateProposal,
               std::unique_ptr<shared_model::interface::Proposal>(
                   shared_model::interface::types::HeightType,
                   shared_model::interface::types::TimestampType,
                   TransactionsCollectionType));
};

struct MockCommonObjectsFactory
    : public shared_model::interface::CommonObjectsFactory {
  MOCK_METHOD2(createPeer,
               FactoryResult<std::unique_ptr<shared_model::interface::Peer>>(
                   const shared_model::interface::types::AddressType &,
                   const shared_model::interface::types::PubkeyType &));

  MOCK_METHOD4(createAccount,
               FactoryResult<std::unique_ptr<shared_model::interface::Account>>(
                   const shared_model::interface::types::AccountIdType &,
                   const shared_model::interface::types::DomainIdType &,
                   shared_model::interface::types::QuorumType,
                   const shared_model::interface::types::JsonType &));

  MOCK_METHOD3(
      createAccountAsset,
      FactoryResult<std::unique_ptr<shared_model::interface::AccountAsset>>(
          const shared_model::interface::types::AccountIdType &,
          const shared_model::interface::types::AssetIdType &,
          const shared_model::interface::Amount &));

  MOCK_METHOD3(createAsset,
               FactoryResult<std::unique_ptr<shared_model::interface::Asset>>(
                   const shared_model::interface::types::AssetIdType &,
                   const shared_model::interface::types::DomainIdType &,
                   shared_model::interface::types::PrecisionType));

  MOCK_METHOD2(createDomain,
               FactoryResult<std::unique_ptr<shared_model::interface::Domain>>(
                   const shared_model::interface::types::DomainIdType &,
                   const shared_model::interface::types::RoleIdType &));

  MOCK_METHOD2(
      createSignature,
      FactoryResult<std::unique_ptr<shared_model::interface::Signature>>(
          const shared_model::interface::types::PubkeyType &,
          const shared_model::interface::Signature::SignedType &));
};

#endif  // IROHA_SHARED_MODEL_INTERFACE_MOCKS_HPP
