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
#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"

using namespace std::chrono_literals;
using namespace iroha::model::generators;

class TransferAssetInterDomainTest : public TxPipelineIntegrationTestFixture {
 public:
  void SetUp() override {
    iroha::ametsuchi::AmetsuchiTest::SetUp();

    // creates node and admin keys and generate default genesis transaction
    auto genesis_tx1 = TransactionGenerator().generateGenesisTransaction(
        0,
        {"0.0.0.0:"
         + std::to_string(TxPipelineIntegrationTestFixture::default_port)});
    // load admin key pair note: generateGenesisTransaction() creates admin key
    // pair
    adminKeypair_ = getVal(iroha::KeysManagerImpl(ADMIN_ID).loadKeys());

    // generate and load NBA, Ivan and Tea key pairs
    nbaKeypair_ = createNewAccountKeypair(NBA_ID);
    ivanKeypair_ = createNewAccountKeypair(IVAN_ID);
    teaKeypair_ = createNewAccountKeypair(TEA_ID);

    // Admin creates domains: [usabnk, khm, ru]
    auto genesis_tx2 = TransactionGenerator().generateTransaction(
        ADMIN_ID,
        2,
        {CommandGenerator().generateCreateDomain(DOMAIN_USABANK,
                                                 USER_DEFAULT_ROLE),
         CommandGenerator().generateCreateDomain(DOMAIN_RU, USER_DEFAULT_ROLE),
         CommandGenerator().generateCreateDomain(DOMAIN_KHM, USER_DEFAULT_ROLE),
         CommandGenerator().generateCreateAccount(
             NBA_NAME, DOMAIN_USABANK, nbaKeypair_.pubkey),
         CommandGenerator().generateAppendRole(NBA_ID, MONEY_CREATOR_ROLE),
         CommandGenerator().generateCreateAccount(
             IVAN_NAME, DOMAIN_RU, ivanKeypair_.pubkey),
         CommandGenerator().generateCreateAccount(
             TEA_NAME, DOMAIN_KHM, teaKeypair_.pubkey)});

    iroha::model::ModelCryptoProviderImpl(adminKeypair_).sign(genesis_tx2);

    genesis_block =
        iroha::model::generators::BlockGenerator().generateGenesisBlock(
            0, {genesis_tx1, genesis_tx2});

    // load node0 key pair
    manager = std::make_shared<iroha::KeysManagerImpl>("node0");
    auto old_keypair = getVal(manager->loadKeys());

    shared_model::crypto::PublicKey publicKey(old_keypair.pubkey.to_string());
    shared_model::crypto::PrivateKey privateKey(
        old_keypair.privkey.to_string());
    shared_model::crypto::Keypair keypair(publicKey, privateKey);

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

    irohad->storage->insertBlock(shared_model::proto::from_old(genesis_block));

    // reset ordering storage state
    irohad->resetOrderingService();

    irohad->init();

    // restore World State View to make sure it is valid
    irohad->restoreWsv();

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
    std::remove("nba@usabnk.pub");
    std::remove("nba@usabnk.priv");
    std::remove("ivan@ru.pub");
    std::remove("ivan@ru.priv");
    std::remove("tea@khm.pub");
    std::remove("tea@khm.priv");
  }

  template <typename T>
  T getVal(boost::optional<T> const &t) {
    EXPECT_TRUE(t);
    return *t;
  }

  const std::string DOMAIN_USABANK = "usabnk";
  const std::string DOMAIN_RU = "ru";
  const std::string DOMAIN_KHM = "khm";
  const std::string USER_DEFAULT_ROLE = "user";
  const std::string MONEY_CREATOR_ROLE = "money_creator";
  const std::string ASSET_USD_NAME = "usd";
  const std::string ASSET_USD_ID = ASSET_USD_NAME + "#" + DOMAIN_USABANK;
  const std::string NBA_NAME = "nba";
  const std::string IVAN_NAME = "ivan";
  const std::string TEA_NAME = "tea";
  const std::string ADMIN_ID = "admin@test";
  const std::string NBA_ID = NBA_NAME + "@" + DOMAIN_USABANK;
  const std::string IVAN_ID = IVAN_NAME + "@" + DOMAIN_RU;
  const std::string TEA_ID = TEA_NAME + "@" + DOMAIN_KHM;

  iroha::keypair_t adminKeypair_;
  iroha::keypair_t nbaKeypair_;
  iroha::keypair_t ivanKeypair_;
  iroha::keypair_t teaKeypair_;
};

/**
 * @given NBA(nba@usabnk) creates asset usd and add asset quantity 1000000$
 * @when A user ivan in domain of Russia (ivan@ru) wants to possess and
 *       transfer asset usd#usabnk to user tea in domain of Cambodia (tea@khm).
 * @then Validate transactions processed.
 */
TEST_F(TransferAssetInterDomainTest, TransferAssetInterDomainTest) {
  // NBA creates usd asset and add own asset quantity.
  auto tx1 = TransactionGenerator().generateTransaction(
      NBA_ID,
      1,
      {CommandGenerator().generateCreateAsset(
           ASSET_USD_NAME, DOMAIN_USABANK, 2),
       CommandGenerator().generateAddAssetQuantity(
           NBA_ID,  // MoneyCreator, who can AddAssetQuantity to my own wallet.
           ASSET_USD_ID,
           getVal(iroha::Amount().createFromString("1000000.00")))});
  iroha::model::ModelCryptoProviderImpl(nbaKeypair_).sign(tx1);

  // NBA transfers asset to ivan@ru and tea@khm
  auto tx2 = TransactionGenerator().generateTransaction(
      NBA_ID,
      1,
      {CommandGenerator().generateTransferAsset(
           NBA_ID,
           IVAN_ID,
           ASSET_USD_ID,
           getVal(iroha::Amount().createFromString("30.50"))),
       CommandGenerator().generateTransferAsset(
           NBA_ID,
           TEA_ID,
           ASSET_USD_ID,
           getVal(iroha::Amount().createFromString("50.00")))});
  iroha::model::ModelCryptoProviderImpl(nbaKeypair_).sign(tx2);

  // transfer asset from ivan@ru to tea@khm (between different domains)
  auto tx3 = TransactionGenerator().generateTransaction(
      IVAN_ID,
      1,
      {CommandGenerator().generateTransferAsset(
          IVAN_ID,
          TEA_ID,
          ASSET_USD_ID,
          getVal(iroha::Amount().createFromString("5.50")))});
  iroha::model::ModelCryptoProviderImpl(ivanKeypair_).sign(tx3);

  sendTxsInOrderAndValidate({tx1, tx2, tx3});
}
