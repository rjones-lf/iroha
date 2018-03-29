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

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"

constexpr auto kUser = "user@test";
constexpr auto kAsset = "asset#domain";
const shared_model::crypto::Keypair kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

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
