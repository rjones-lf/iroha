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

#include <gtest/gtest.h>
#include <model/model_hash_provider_impl.hpp>
#include "commands.pb.h"
#include "model/block.hpp"
#include "model/converters/pb_block_factory.hpp"

#include "model/commands/add_asset_quantity.hpp"
#include "model/commands/add_peer.hpp"
#include "model/commands/add_signatory.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "model/commands/remove_signatory.hpp"
#include "model/commands/set_permissions.hpp"
#include "model/commands/set_quorum.hpp"
#include "model/commands/transfer_asset.hpp"

TEST(BlockTest, bl_test) {
  auto orig_tx = iroha::model::Transaction();
  orig_tx.creator_account_id = "andr@kek";
  auto siga = iroha::model::Signature();
  std::fill(siga.pubkey.begin(), siga.pubkey.end(), 0x22);
  std::fill(siga.signature.begin(), siga.signature.end(), 0x10);
  orig_tx.signatures = {siga};

  orig_tx.created_ts = 2;
  orig_tx.tx_counter = 1;

  auto c1 = iroha::model::CreateDomain();
  c1.domain_name = "keker";
  auto c2 = iroha::model::CreateAsset();
  c2.domain_id = "keker";
  c2.precision = 2;
  c2.asset_name = "fedor-coin";

  auto c3 = iroha::model::SetAccountPermissions();
  c3.account_id = "fedor";
  c3.new_permissions.can_transfer = true;
  c3.new_permissions.create_assets = true;

  orig_tx.commands = {
      std::make_shared<iroha::model::CreateDomain>(c1),
      std::make_shared<iroha::model::CreateAsset>(c2),
      std::make_shared<iroha::model::SetAccountPermissions>(c3)};

  auto orig_block = iroha::model::Block();
  orig_block.created_ts = 1;

  std::fill(orig_block.prev_hash.begin(), orig_block.prev_hash.end(), 0x3);
  orig_block.sigs = {siga};

  std::fill(orig_block.merkle_root.begin(), orig_block.merkle_root.end(), 0x14);
  orig_block.height = 3;
  orig_block.txs_number = 1;
  orig_block.transactions = {orig_tx};

  iroha::model::HashProviderImpl hash_provider;
  orig_block.hash = hash_provider.get_hash(orig_block);

  auto factory = iroha::model::converters::PbBlockFactory();
  auto proto_block = factory.serialize(orig_block);
  auto serial_block = factory.deserialize(proto_block);
  ASSERT_EQ(orig_block.created_ts, serial_block.created_ts);
  ASSERT_EQ(orig_block.transactions, serial_block.transactions);
  ASSERT_EQ(orig_block.sigs, serial_block.sigs);
  ASSERT_EQ(orig_block.hash, serial_block.hash);
  ASSERT_EQ(orig_block.merkle_root, serial_block.merkle_root);
  ASSERT_EQ(orig_block.prev_hash, serial_block.prev_hash);
  ASSERT_EQ(orig_block.txs_number, serial_block.txs_number);
  ASSERT_EQ(orig_block, serial_block);
}
