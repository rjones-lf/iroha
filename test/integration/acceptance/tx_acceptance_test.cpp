/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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


#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"
#include "model/generators/query_generator.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "responses.pb.h"

constexpr auto kAdmin = "admin@test";
constexpr auto kNonUser = "nonuser@test";
constexpr auto kAsset = "coin#test";
const shared_model::crypto::Keypair kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
auto checkStatelessValid = [](auto &status) {
  ASSERT_NO_THROW(
      boost::get<shared_model::detail::PolymorphicWrapper<
          shared_model::interface::StatelessValidTxResponse>>(status.get()));
};
auto checkStatelessInvalid = [](auto &status) {
  ASSERT_NO_THROW(
      boost::get<shared_model::detail::PolymorphicWrapper<
          shared_model::interface::StatelessFailedTxResponse>>(status.get()));
};
auto checkProposal = [](auto &proposal) {
  ASSERT_EQ(proposal->transactions().size(), 1);

};
auto checkStatefulInvalid = [](auto &block) {
  ASSERT_EQ(block->transactions().size(), 0);
};
auto checkStatefulValid = [](auto &block) {
  ASSERT_EQ(block->transactions().size(), 1);
};

/**
 * @given non existent user
 * @when sending  transaction to the ledger
 * @then receive STATELESS_VALIDATION_SUCCESS status
 *       AND STATEFUL_VALIDATION_FAILED on that tx
 */
TEST(AcceptanceTest, NonExistentCreatorAccountId) {
  auto tx = shared_model::proto::TransactionBuilder()
                .txCounter(2)
                .createdTime(iroha::time::now())
                .creatorAccountId(kNonUser)
                .addAssetQuantity(kAdmin, kAsset, "1.0")
                .build()
                .signAndAddSignature(kAdminKeypair);

  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessValid)
      .checkProposal(checkProposal)
      .checkBlock(checkStatefulInvalid)
      .done();
}

/**
 * @given some user
 * @when sending 2 transactions with the same txCounter to the ledger
 * @then receive STATELESS_VALIDATION_SUCCESS status on both txs
 *       AND STATELESS_VALIDATION_SUCCESS on first
 *       AND STATEFUL_VALIDATION_FAILED on second
 */

// TODO: Solonets / IR-1166 - Double tx counter / Satisfy disabled test
TEST(AcceptanceTest, DISABLED_DublicatedTxCounter) {
  auto tx1 = shared_model::proto::TransactionBuilder()
                 .txCounter(2)
                 .createdTime(iroha::time::now())
                 .creatorAccountId(kAdmin)
                 .addAssetQuantity(kAdmin, kAsset, "1.0")
                 .build()
                 .signAndAddSignature(kAdminKeypair);
  auto tx2 = shared_model::proto::TransactionBuilder()
                 .txCounter(2)
                 .createdTime(iroha::time::now())
                 .creatorAccountId(kAdmin)
                 .addAssetQuantity(kAdmin, kAsset, "1.0")
                 .build()
                 .signAndAddSignature(kAdminKeypair);

  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx1, checkStatelessValid)
      .skipProposal()
      .checkBlock(checkStatefulValid)
      .sendTx(tx2, checkStatelessValid)
      .skipProposal()
      .checkBlock(checkStatefulInvalid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with the Maximum txCounter possible to the ledger
 * @then receive STATELESS_VALIDATION_SUCCESS status
 *       AND STATEFUL_VALIDATION_SUCCESS on that tx
 */
TEST(AcceptanceTest, MaxTxCounter) {
  auto tx1 = shared_model::proto::TransactionBuilder()
                 .txCounter(2)
                 .createdTime(iroha::time::now())
                 .creatorAccountId(kAdmin)
                 .addAssetQuantity(kAdmin, kAsset, "1.0")
                 .build()
                 .signAndAddSignature(kAdminKeypair);

  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx1, checkStatelessValid)
      .skipProposal()
      .checkBlock(checkStatefulValid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with an 1 hour old UNIX time
 * @then receive STATELESS_VALIDATION_SUCCESS status
 *       AND STATEFUL_VALIDATION_SUCCESS on that tx
 */
TEST(AcceptanceTest, Transaction1HourOld) {
  auto tx = shared_model::proto::TransactionBuilder()
                .txCounter(2)
                .createdTime(iroha::time::now(std::chrono::hours(-1)))
                .creatorAccountId(kAdmin)
                .addAssetQuantity(kAdmin, kAsset, "1.0")
                .build()
                .signAndAddSignature(kAdminKeypair);
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessValid)
      .skipProposal()
      .checkBlock(checkStatefulValid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with an less than 24 hour old UNIX time
 * @then receive STATELESS_VALIDATION_SUCCESS status
 *       AND STATEFUL_VALIDATION_SUCCESS on that tx
 */
TEST(AcceptanceTest, DISABLED_TransactionLess24HourOld) {
  auto tx = shared_model::proto::TransactionBuilder()
                .txCounter(2)
                .createdTime(iroha::time::now(std::chrono::hours(24)
                                              - std::chrono::minutes(1)))
                .creatorAccountId(kAdmin)
                .addAssetQuantity(kAdmin, kAsset, "1.0")
                .build()
                .signAndAddSignature(kAdminKeypair);
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessValid)
      .skipProposal()
      .checkBlock(checkStatefulValid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with an more than 24 hour old UNIX time
 * @then receive STATELESS_VALIDATION_FAILED status
 */
TEST(AcceptanceTest, TransactionMore24HourOld) {
  ASSERT_ANY_THROW(
      auto tx = shared_model::proto::TransactionBuilder()
                    .txCounter(2)
                    .createdTime(iroha::time::now(std::chrono::hours(24)
                                                  + std::chrono::minutes(1)))
                    .creatorAccountId(kAdmin)
                    .addAssetQuantity(kAdmin, kAsset, "1.0")
                    .build()
                    .signAndAddSignature(kAdminKeypair););
  auto tx = TestUnsignedTransactionBuilder()
                .txCounter(2)
                .createdTime(iroha::time::now(std::chrono::hours(24)
                                              + std::chrono::minutes(1)))
                .creatorAccountId(kAdmin)
                .addAssetQuantity(kAdmin, kAsset, "1.0")
                .build()
                .signAndAddSignature(kAdminKeypair);
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessInvalid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with an less that 5 minutes from future UNIX time
 * @then receive STATELESS_VALIDATION_SUCCESS status
 *       AND STATEFUL_VALIDATION_SUCCESS on that tx
 */
TEST(AcceptanceTest, Transaction5MinutesFromFuture) {
  auto tx = shared_model::proto::TransactionBuilder()
                .txCounter(2)
                .createdTime(iroha::time::now(std::chrono::minutes(5)
                                              - std::chrono::seconds(10)))
                .creatorAccountId(kAdmin)
                .addAssetQuantity(kAdmin, kAsset, "1.0")
                .build()
                .signAndAddSignature(kAdminKeypair);

  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessValid)
      .skipProposal()
      .checkBlock(checkStatefulValid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with an 10 minutes from future UNIX time
 * @then receive STATELESS_VALIDATION_FAILED status
 */
TEST(AcceptanceTest, Transaction10MinutesFromFuture) {
  ASSERT_ANY_THROW(
      auto tx = shared_model::proto::TransactionBuilder()
                    .txCounter(2)
                    .createdTime(iroha::time::now(std::chrono::minutes(10)))
                    .creatorAccountId(kAdmin)
                    .addAssetQuantity(kAdmin, kAsset, "1.0")
                    .build()
                    .signAndAddSignature(kAdminKeypair););
  auto tx = TestUnsignedTransactionBuilder()
                .txCounter(2)
                .createdTime(iroha::time::now(std::chrono::minutes(10)))
                .creatorAccountId(kAdmin)
                .addAssetQuantity(kAdmin, kAsset, "1.0")
                .build()
                .signAndAddSignature(kAdminKeypair);
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessInvalid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with an empty public Key
 * @then receive STATELESS_VALIDATION_FAILED status
 */
TEST(AcceptanceTest, TransactionEmptyPubKey) {
  shared_model::proto::Transaction tx =
      TestTransactionBuilder()
          .txCounter(2)
          .createdTime(iroha::time::now())
          .creatorAccountId(kAdmin)
          .addAssetQuantity(kAdmin, kAsset, "1.0")
          .build();

  auto signedBlob = shared_model::crypto::CryptoSigner<>::sign(
      shared_model::crypto::Blob(tx.payload()), kAdminKeypair);
  iroha::protocol::Signature protosig;
  protosig.set_pubkey(shared_model::crypto::toBinaryString(
      shared_model::crypto::PublicKey("")));
  protosig.set_signature("");
  auto *s1 = new shared_model::proto::Signature(protosig);
  tx.addSignature(
      shared_model::detail::PolymorphicWrapper<shared_model::proto::Signature>(
          s1));
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessInvalid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with an empty signedBlob
 * @then receive STATELESS_VALIDATION_FAILED status
 */
TEST(AcceptanceTest, TransactionEmptySignedblob) {
  shared_model::proto::Transaction tx =
      TestTransactionBuilder()
          .txCounter(2)
          .createdTime(iroha::time::now())
          .creatorAccountId(kAdmin)
          .addAssetQuantity(kAdmin, kAsset, "1.0")
          .build();
  iroha::protocol::Signature protosig;
  protosig.set_pubkey(
      shared_model::crypto::toBinaryString(kAdminKeypair.publicKey()));
  protosig.set_signature(
      shared_model::crypto::toBinaryString(shared_model::crypto::Blob("")));
  auto *s1 = new shared_model::proto::Signature(protosig);
  tx.addSignature(
      shared_model::detail::PolymorphicWrapper<shared_model::proto::Signature>(
          s1));
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessInvalid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with Invalid PublicKey
 * @then receive STATELESS_VALIDATION_FAILED status
 */
TEST(AcceptanceTest, TransactionInvalidPublicKey) {
  shared_model::proto::Transaction tx =
      TestTransactionBuilder()
          .txCounter(2)
          .createdTime(iroha::time::now())
          .creatorAccountId(kAdmin)
          .addAssetQuantity(kAdmin, kAsset, "1.0")
          .build();
  auto signedBlob = shared_model::crypto::CryptoSigner<>::sign(
      shared_model::crypto::Blob(tx.payload()), kAdminKeypair);
  iroha::protocol::Signature protosig;
  protosig.set_pubkey(shared_model::crypto::toBinaryString(
      shared_model::crypto::PublicKey(std::string(32, 'a'))));
  protosig.set_signature(shared_model::crypto::toBinaryString(signedBlob));
  auto *s1 = new shared_model::proto::Signature(protosig);
  tx.addSignature(
      shared_model::detail::PolymorphicWrapper<shared_model::proto::Signature>(
          s1));
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessInvalid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with Invalid SignedBlock
 * @then receive STATELESS_VALIDATION_FAILED status
 */
TEST(AcceptanceTest, TransactionInvalidSignedBlob) {
  shared_model::proto::Transaction tx =
      TestTransactionBuilder()
          .txCounter(2)
          .createdTime(iroha::time::now())
          .creatorAccountId(kAdmin)
          .addAssetQuantity(kAdmin, kAsset, "1.0")
          .build();
  auto signedBlob = shared_model::crypto::CryptoSigner<>::sign(
      shared_model::crypto::Blob(tx.payload()), kAdminKeypair);
  iroha::protocol::Signature protosig;
  protosig.set_pubkey(
      shared_model::crypto::toBinaryString(kAdminKeypair.publicKey()));
  auto raw = signedBlob.blob();
  raw[0] = (raw[0] == std::numeric_limits<uint8_t>::max() ? 0 : raw[0] + 1);
  auto wrongBlob = shared_model::crypto::Blob(raw);
  protosig.set_signature(shared_model::crypto::toBinaryString(wrongBlob));
  auto *s1 = new shared_model::proto::Signature(protosig);
  tx.addSignature(
      shared_model::detail::PolymorphicWrapper<shared_model::proto::Signature>(
          s1));
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessInvalid)
      .done();
}

/**
 * @given some user
 * @when sending transactions with valid signature
 * @then receive STATELESS_VALIDATION_SUCCESS status
 *       AND STATEFUL_VALIDATION_SUCCESS on that tx
 */
TEST(AcceptanceTest, TransactionValidSignedBlob) {
  shared_model::proto::Transaction tx =
      shared_model::proto::TransactionBuilder()
          .txCounter(2)
          .createdTime(iroha::time::now())
          .creatorAccountId(kAdmin)
          .addAssetQuantity(kAdmin, kAsset, "1.0")
          .build()
          .signAndAddSignature(kAdminKeypair);
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessValid)
      .skipProposal()
      .checkBlock(checkStatefulValid)
      .done();
}
