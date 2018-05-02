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
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/base_tx.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "interfaces/utils/specified_visitor.hpp"
#include "validators/permissions.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class MstPipelineTest : public testing::Test {
 public:
  /**
   * @param tx pre-built transaction
   * @param signatory id of signatory
   * @return signed transaction
   */
  template <typename TxBuilder>
  auto signTx(TxBuilder tx, const crypto::Keypair &key) const {
    return tx.build().signAndAddSignature(key);
  }

  /**
   * Create valid base pre-built transaction
   * @return pre-built tx
   */
  auto baseTx() const {
    return proto::TransactionBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId(kUserId)
        .addAssetQuantity(kUserId, kAsset, "1.0");
  }

  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeMstUser(size_t sigs = kSignatories) {
    auto tx = framework::createUserWithPerms(
                  kUser,
                  kUserKeypair.publicKey(),
                  kNewRole,
                  std::vector<std::string>{
                      shared_model::permissions::can_add_asset_qty})
                  .setAccountQuorum(kUserId, sigs + 1);

    for (size_t i = 0; i < sigs; ++i) {
      signatories.emplace_back(
          crypto::DefaultCryptoAlgorithmType::generateKeypair());
      tx = tx.addSignatory(kUserId, signatories[i].publicKey());
    }

    return tx.build().signAndAddSignature(kAdminKeypair);
  }

  const std::string kUser = "user"s;
  const std::string kNewRole = "rl"s;
  const std::string kUserId = kUser + "@test";
  const std::string kAsset = "asset#domain";
  static const size_t kSignatories = 2;
  std::vector<crypto::Keypair> signatories;
  const crypto::Keypair kAdminKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
  const crypto::Keypair kUserKeypair =
      crypto::DefaultCryptoAlgorithmType::generateKeypair();
};

/**
 * @given multisignature account, pair of signers
 *        AND tx with an AddAssetQuantity command
 * @when sending with author signature and then with signers' one
 * @then firstly there's no commit then it is
 */
TEST_F(MstPipelineTest, OnePeerSendsTest) {
  const size_t kProposalSize = 10;
  auto tx = baseTx().quorum(kSignatories + 1);

  IntegrationTestFramework itf(kProposalSize, [](auto &i) { i.done(); }, true);
  itf.setInitialState(kAdminKeypair)
      .sendTx(makeMstUser())
      .skipProposal()
      .skipBlock()
      .sendTx(signTx(tx, kUserKeypair));
  ASSERT_ANY_THROW(itf.skipProposal());
  itf.sendTx(signTx(tx, signatories.at(0)))
      .sendTx(signTx(tx, signatories.at(1)))
      .skipProposal()
      .skipBlock();
}
