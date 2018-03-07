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
#define BOOST_VARIANT_USE_RELAXED_GET_BY_DEFAULT
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/pipeline/tx_pipeline_integration_test_fixture.hpp"

TEST(QueryAcceptanceTest, TransactionValidSignedBlob) {
  auto keypair =
      shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
  shared_model::proto::Transaction tx =
      shared_model::proto::TransactionBuilder()
          .txCounter(2)
          .createdTime(iroha::time::now())
          .creatorAccountId("admin@test")
          .addAssetQuantity("admin@test", "coin#test", "1.0")
          .build()
          .signAndAddSignature(keypair);
  std::vector<shared_model::crypto::Hash> hashes = {tx.hash()};
  shared_model::proto::Query q = shared_model::proto::QueryBuilder()
                                     .creatorAccountId("admin@test")
                                     .queryCounter(1)
                                     .createdTime(iroha::time::now())
                                     .getTransactions(hashes)
                                     .build()
                                     .signAndAddSignature(keypair);
  integration_framework::IntegrationTestFramework(1)
      .setInitialState(keypair)
      .sendTx(tx)
      .skipProposal()
      .skipBlock()
      .sendQuery(q, [](shared_model::proto::QueryResponse response){
        auto tr =  boost::get<shared_model::detail::PolymorphicWrapper<
          shared_model::interface::TransactionsResponse>>(response.get());
        ASSERT_EQ(tr->transactions().size(), 1);
      })
      .done();
}
