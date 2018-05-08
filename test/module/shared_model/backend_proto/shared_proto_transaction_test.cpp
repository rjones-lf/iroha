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

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "cryptography/ed25519_sha3_impl/crypto_provider.hpp"
#include "utils/polymorphic_wrapper.hpp"

#include <gtest/gtest.h>

// common data for tests
auto created_time = iroha::time::now();
std::string creator_account_id = "admin@test";

/**
 * generates sample transaction without commands
 * @return generated transaction
 */
iroha::protocol::Transaction generateEmptyTransaction() {
  iroha::protocol::Transaction proto_tx;
  auto &payload = *proto_tx.mutable_payload();
  payload.set_creator_account_id(creator_account_id);
  payload.set_created_time(created_time);

  return proto_tx;
}

/**
 * Helper function to generate AddAssetQuantityCommand
 * @param account_id account id to add asset quantity to
 * @param asset_id asset id to add value to
 * @return AddAssetQuantity protocol command
 */
iroha::protocol::AddAssetQuantity generateAddAssetQuantity(
    std::string account_id, std::string asset_id) {
  iroha::protocol::AddAssetQuantity command;

  command.set_account_id(account_id);
  command.set_asset_id(asset_id);
  command.mutable_amount()->mutable_value()->set_fourth(1000);
  command.mutable_amount()->set_precision(2);

  return command;
}

/**
 * @given transaction field values and sample command values, reference tx
 * @when create transaction with sample command using transaction builder
 * @then transaction is built correctly
 */
TEST(ProtoTransaction, Builder) {
  iroha::protocol::Transaction proto_tx = generateEmptyTransaction();

  std::string account_id = "admin@test", asset_id = "coin#test",
              amount = "10.00";
  auto command =
      proto_tx.mutable_payload()->add_commands()->mutable_add_asset_quantity();

  command->CopyFrom(generateAddAssetQuantity(account_id, asset_id));

  auto keypair =
      shared_model::crypto::CryptoProviderEd25519Sha3::generateKeypair();
  auto signedProto = shared_model::crypto::CryptoSigner<>::sign(
      shared_model::crypto::Blob(proto_tx.payload().SerializeAsString()),
      keypair);

  auto sig = proto_tx.add_signatures();
  sig->set_pubkey(shared_model::crypto::toBinaryString(keypair.publicKey()));
  sig->set_signature(shared_model::crypto::toBinaryString(signedProto));

  auto tx = shared_model::proto::TransactionBuilder()
                .creatorAccountId(creator_account_id)
                .addAssetQuantity(account_id, asset_id, amount)
                .createdTime(created_time)
                .build();

  auto signedTx = tx.signAndAddSignature(keypair);
  auto &proto = signedTx.getTransport();

  ASSERT_EQ(proto_tx.SerializeAsString(), proto.SerializeAsString());
}

/**
 * @given transaction field values and sample command values with wrongly set
 * values
 * @when create transaction with sample command using transaction builder
 * @then transaction throws exception due to badly formed fields in commands
 */
TEST(ProtoTransaction, BuilderWithInvalidTx) {
  std::string invalid_account_id = "admintest";  // invalid account_id without @
  std::string invalid_asset_id = "cointest",     // invalid asset_id without #
      amount = "10.00";

  ASSERT_THROW(
      shared_model::proto::TransactionBuilder()
          .creatorAccountId(invalid_account_id)
          .addAssetQuantity(invalid_account_id, invalid_asset_id, amount)
          .createdTime(created_time)
          .build(),
      std::invalid_argument);
}
