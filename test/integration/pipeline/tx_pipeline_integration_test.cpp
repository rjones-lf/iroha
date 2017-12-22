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

#include "responses.pb.h"
#include "datetime/time.hpp"
#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"
#include "model/generators/query_generator.hpp"

using namespace iroha::model::generators;
using namespace iroha::model::converters;

// TODO: refactor services to allow dynamic port binding IR-741
class TxPipelineIntegrationTest : public TxPipelineIntegrationTestFixture {
 public:
  void SetUp() override {
    iroha::ametsuchi::AmetsuchiTest::SetUp();

    auto genesis_tx =
        TransactionGenerator().generateGenesisTransaction(0, {"0.0.0.0:10001"});
    genesis_block =
        iroha::model::generators::BlockGenerator().generateGenesisBlock(
            0, {genesis_tx});

    manager = std::make_shared<iroha::KeysManagerImpl>("node0");
    auto keypair = manager->loadKeys().value();

    irohad = std::make_shared<TestIrohad>(block_store_path,
                                          redishost_,
                                          redisport_,
                                          pgopt_,
                                          0,
                                          10001,
                                          10,
                                          5000ms,
                                          5000ms,
                                          5000ms,
                                          keypair);

    ASSERT_TRUE(irohad->storage);

    // insert genesis block
    iroha::main::BlockInserter inserter(irohad->storage);

    inserter.applyToLedger({genesis_block});

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
  //TODO 19/12/17 motxx - Rework integration test using shared model (IR-715 comment)
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
  iroha::model::ModelCryptoProviderImpl provider(keypair);
  provider.sign(tx);

  sendTxsInOrderAndValidate({tx});
}

/**
 * @given Admin sends some transaction and keep its hash
 * @when GetTransactions query with the hash is sent
 * @then Validate the transaction
 */
TEST_F(TxPipelineIntegrationTest, GetTransactionsTest) {
  //TODO 19/12/17 motxx - Rework integration test using shared model (IR-715 comment)
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
  iroha::model::ModelCryptoProviderImpl provider(keypair);
  provider.sign(given_tx);

  sendTxsInOrderAndValidate({given_tx});

  // keep sent tx's hash
  const auto given_tx_hash = iroha::hash(given_tx);

  auto query =
      iroha::model::generators::QueryGenerator().generateGetTransactions(
        iroha::time::now(), CREATOR_ACCOUNT_ID, 1, {given_tx_hash});
  provider.sign(*query);

  const auto pb_query = PbQueryFactory{}.serialize(query);
  ASSERT_TRUE(pb_query.has_value());

  iroha::protocol::QueryResponse response;
  irohad->getQueryService()->FindAsync(pb_query.value(), response);
  ASSERT_EQ(1, response.transactions_response().transactions().size());
  const auto got_pb_tx = response.transactions_response().transactions()[0];
  ASSERT_EQ(given_tx, *PbTransactionFactory{}.deserialize(got_pb_tx));
}
