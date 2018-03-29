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

#include "backend/protobuf/from_old_model.hpp"
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"
#include "model/generators/query_generator.hpp"
#include "model/sha3_hash.hpp"
#include "responses.pb.h"

using namespace std::chrono_literals;
using namespace iroha::model::generators;
using namespace iroha::model::converters;

// TODO: refactor services to allow dynamic port binding IR-741
class TxPipelineIntegrationTest : public TxPipelineIntegrationTestFixture {
 public:
  void SetUp() override {
    iroha::ametsuchi::AmetsuchiTest::SetUp();

    auto genesis_tx = TransactionGenerator().generateGenesisTransaction(
        0, {"0.0.0.0:" + std::to_string(default_port)});
    genesis_block =
        iroha::model::generators::BlockGenerator().generateGenesisBlock(
            0, {genesis_tx});

    manager = std::make_shared<iroha::KeysManagerImpl>("node0");
    auto old_keypair = manager->loadKeys().value();
    shared_model::crypto::Keypair keypair(
        shared_model::crypto::PublicKey(old_keypair.pubkey.to_string()),
        shared_model::crypto::PrivateKey(old_keypair.privkey.to_string()));

    irohad = std::make_shared<TestIrohad>(block_store_path,
                                          pgopt_,
                                          0,
                                          default_port,
                                          10,
                                          5000ms,
                                          5000ms,
                                          5000ms,
                                          keypair);

    ASSERT_TRUE(irohad->storage);

    // insert genesis block
    irohad->storage->insertBlock(shared_model::proto::from_old(genesis_block));

    // reset ordering storage state
    irohad->resetOrderingService();

    // initialize irohad
    irohad->init();

    // start irohad
    irohad->run();
  }

  void TearDown() override {
    iroha::ametsuchi::AmetsuchiTest::TearDown();
    std::remove("node0.pub");
    std::remove("node0.priv");
    std::remove("admin@test.pub");
    std::remove("admin@test.priv");
    std::remove("test@test.pub");
    std::remove("test@test.priv");
  }
};

TEST_F(TxPipelineIntegrationTest, TxPipelineTest) {
  // TODO 19/12/17 motxx - Rework integration test using shared model (IR-715
  // comment)
  // generate test command
  auto cmd =
      iroha::model::generators::CommandGenerator().generateAddAssetQuantity(
          "admin@test",
          "coin#test",
          iroha::Amount().createFromString("20.00").value());

  // generate test transaction
  auto tx =
      iroha::model::generators::TransactionGenerator().generateTransaction(
          "admin@test", 1, {cmd});
  iroha::KeysManagerImpl manager("admin@test");
  auto keypair = manager.loadKeys().value();

  shared_model::crypto::Keypair keypair_(
      shared_model::crypto::PublicKey(keypair.pubkey.to_string()),
      shared_model::crypto::PrivateKey(keypair.privkey.to_string()));
  shared_model::crypto::CryptoModelSigner<> signer(keypair_);
  auto transaction = shared_model::proto::from_old(tx);
  signer.sign(transaction);
  tx = *std::unique_ptr<iroha::model::Transaction>(transaction.makeOldModel());

  sendTxsInOrderAndValidate({tx});
}

/**
 * @given Admin sends some transaction and keep its hash
 * @when GetTransactions query with the hash is sent
 * @then Validate the transaction
 */
TEST_F(TxPipelineIntegrationTest, GetTransactionsTest) {
  // TODO 19/12/17 motxx - Rework integration test using shared model (IR-715
  // comment)
  const auto CREATOR_ACCOUNT_ID = "admin@test";
  // send some transaction
  const auto cmd =
      iroha::model::generators::CommandGenerator().generateAddAssetQuantity(
          CREATOR_ACCOUNT_ID,
          "coin#test",
          iroha::Amount().createFromString("20.00").value());
  auto given_tx =
      iroha::model::generators::TransactionGenerator().generateTransaction(
          CREATOR_ACCOUNT_ID, 1, {cmd});
  iroha::KeysManagerImpl manager(CREATOR_ACCOUNT_ID);
  const auto keypair = manager.loadKeys().value();

  shared_model::crypto::Keypair keypair_(
      shared_model::crypto::PublicKey(keypair.pubkey.to_string()),
      shared_model::crypto::PrivateKey(keypair.privkey.to_string()));
  shared_model::crypto::CryptoModelSigner<> signer(keypair_);
  auto given_transaction = shared_model::proto::from_old(given_tx);
  signer.sign(given_transaction);
  given_tx = *std::unique_ptr<iroha::model::Transaction>(given_transaction.makeOldModel());

  sendTxsInOrderAndValidate({given_tx});

  // keep sent tx's hash
  const auto given_tx_hash = iroha::hash(given_tx);

  auto query =
      iroha::model::generators::QueryGenerator().generateGetTransactions(
          iroha::time::now(), CREATOR_ACCOUNT_ID, 1, {given_tx_hash});
  auto query_ = shared_model::proto::from_old(query);
  signer.sign(query_);

  const auto pb_query = PbQueryFactory{}.serialize(query);
  ASSERT_TRUE(pb_query);

  iroha::protocol::QueryResponse response;
  irohad->getQueryService()->Find(*pb_query, response);
  ASSERT_EQ(1, response.transactions_response().transactions().size());
  const auto got_pb_tx = response.transactions_response().transactions()[0];
  ASSERT_EQ(given_tx, *PbTransactionFactory{}.deserialize(got_pb_tx));
}

constexpr auto kUser = "user@test";
constexpr auto kAsset = "asset#domain";

const auto kAdminOldKeypair = iroha::create_keypair();
const shared_model::crypto::Keypair kAdminKeypair(
    shared_model::crypto::PublicKey(kAdminOldKeypair.pubkey.to_string()),
    shared_model::crypto::PrivateKey(kAdminOldKeypair.privkey.to_string()));

/**
 * @given GetAccount query
 * AND default-initialized IntegrationTestFramework
 * @when query is sent to the framework
 * @then query response is ErrorResponse with STATEFUL_INVALID reason
 */
TEST(PipelineIntegrationTest, SendQuery) {
  auto query = shared_model::proto::QueryBuilder()
                   .createdTime(iroha::time::now())
                   .creatorAccountId(kUser)
                   .queryCounter(1)
                   .getAccount(kUser)
                   .build()
                   .signAndAddSignature(
                       shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair());

  auto check = [](auto &status) {
    using ExpectedResponseType = shared_model::detail::PolymorphicWrapper<
        shared_model::interface::ErrorQueryResponse>;
    using ExpectedReasonType = shared_model::detail::PolymorphicWrapper<
        shared_model::interface::StatefulFailedErrorResponse>;
    ASSERT_NO_THROW(boost::get<ExpectedResponseType>(status.get()));
    ASSERT_NO_THROW(boost::get<ExpectedReasonType>(
        boost::get<ExpectedResponseType>(status.get())->get()));
  };
  integration_framework::IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendQuery(query, check)
      .done();
}

/**
 * @given some user
 * @when sending sample AddAssetQuantity transaction to the ledger
 * @then receive STATELESS_VALIDATION_SUCCESS status on that tx
 * @and wait for proposal and block
 */
TEST(PipelineIntegrationTest, SendTx) {
  auto tx = shared_model::proto::TransactionBuilder()
                .createdTime(iroha::time::now())
                .creatorAccountId(kUser)
                .txCounter(1)
                .addAssetQuantity(kUser, kAsset, "1.0")
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair());

  auto checkStatelessValid = [](auto &status) {
    ASSERT_NO_THROW(
        boost::get<shared_model::detail::PolymorphicWrapper<
            shared_model::interface::StatelessValidTxResponse>>(status.get()));
  };
  auto checkProposal = [](auto &proposal) {
    ASSERT_EQ(proposal->transactions().size(), 1);
  };
  auto checkBlock = [](auto &block) {
    ASSERT_EQ(block->transactions().size(), 0);
  };
  integration_framework::IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(tx, checkStatelessValid)
      .checkProposal(checkProposal)
      .checkBlock(checkBlock)
      .done();
}
