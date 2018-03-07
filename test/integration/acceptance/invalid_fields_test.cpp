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
#include "block.pb.h"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "interfaces/utils/specified_visitor.hpp"
#include "module/shared_model/validators/validators.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class InvalidField : public ::testing::Test {
 public:
  const std::string kUser = "user"s;
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given tx with CreateAccount command and invalid signature size
 * @when send it
 * @then Torii returns stateless fail
 */
TEST_F(InvalidField, Signature) {
  auto tx = proto::TransactionBuilder()
                .createAccount(kUser, "test", kUserKeypair.publicKey())
                .txCounter(1)
                .creatorAccountId("admin@test")
                .createdTime(iroha::time::now())
                .build()
                .signAndAddSignature(kAdminKeypair)
                .getTransport();
  // extend signature to invalid size
  auto sig = tx.mutable_signature(0)->mutable_signature();
  sig->resize(sig->size() + 1, 'a');
  auto check = [](auto &resp) {
    ASSERT_TRUE(boost::apply_visitor(
        interface::SpecifiedVisitor<interface::StatelessFailedTxResponse>(),
        resp.get()));
  };

  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(proto::Transaction(tx), check)
      .done();
}

/**
 * @given tx with CreateAccount command and invalid pub key size
 * @when send it
 * @then Torii returns stateless fail
 */
TEST_F(InvalidField, Pubkey) {
  auto tx = proto::TransactionBuilder()
                .createAccount(kUser, "test", kUserKeypair.publicKey())
                .txCounter(1)
                .creatorAccountId("admin@test")
                .createdTime(iroha::time::now())
                .build()
                .signAndAddSignature(kAdminKeypair)
                .getTransport();
  // extend public key to invalid size
  auto pkey = tx.mutable_signature(0)->mutable_pubkey();
  pkey->resize(pkey->size() + 1, 'a');
  auto check = [](auto &resp) {
    ASSERT_TRUE(boost::apply_visitor(
        interface::SpecifiedVisitor<interface::StatelessFailedTxResponse>(),
        resp.get()));
  };

  IntegrationTestFramework()
      .setInitialState(kAdminKeypair)
      .sendTx(proto::Transaction(tx), check)
      .done();
}
