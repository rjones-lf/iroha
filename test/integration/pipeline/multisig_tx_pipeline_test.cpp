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
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "validators/permissions.hpp"

using namespace std::string_literals;
using namespace integration_framework;
using namespace shared_model;

class MstPipelineTest : public AcceptanceFixture {
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
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeMstUser(size_t sigs = kSignatories) {
    auto tx =
        createUserWithPerms(kUser,
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

  const std::string kNewRole = "rl"s;
  static const size_t kSignatories = 2;
  std::vector<crypto::Keypair> signatories;
};

/**
 * @given multisignature account, pair of signers
 *        AND tx with an AddAssetQuantity command
 * @when sending with author signature and then with signers' one
 * @then firstly there's no commit then it is
 */
TEST_F(MstPipelineTest, OnePeerSendsTest) {
  auto tx =
      baseTx().setAccountQuorum(kUserId, kSignatories).quorum(kSignatories + 1);
  auto user_tx = makeMstUser();

  IntegrationTestFramework(1, [](auto &i) { i.done(); }, true)
      .setInitialState(kAdminKeypair)
      .sendTx(user_tx)
      .skipBlock()
      .sendTx(signTx(tx, kUserKeypair))
      // TODO(@l4l) 21/05/18 IR-1339
      // tx should be checked for MST_AWAIT status
      .sendTx(signTx(tx, signatories.at(0)))
      .sendTx(signTx(tx, signatories.at(1)))
      .skipBlock();
}
