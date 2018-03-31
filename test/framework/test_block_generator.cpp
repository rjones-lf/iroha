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

#include "framework/test_block_generator.hpp"
#include <chrono>
#include <model/commands/create_role.hpp>
#include "model/commands/add_peer.hpp"
#include "model/commands/create_account.hpp"
#include "model/commands/create_asset.hpp"
#include "model/commands/create_domain.hpp"
#include "validators/permissions.hpp"
#include "model/sha3_hash.hpp"

using namespace iroha;
using namespace iroha::model;

namespace framework {
  namespace generator {
    Transaction getAddPeerTransaction(uint64_t create_time,
                                      std::string address) {
      Transaction transaction;
      transaction.created_ts = create_time;
      Signature sign{};
      transaction.signatures = {sign};

      auto add_peer = std::make_shared<AddPeer>();
      add_peer->peer.address = address;
      add_peer->peer.pubkey = {};

      transaction.commands = {add_peer};
      return transaction;
    }

    Transaction getTestCreateTransaction(uint64_t create_time) {
      Transaction transaction;
      transaction.created_ts = create_time;
      Signature sign{};
      transaction.signatures = {sign};

      auto create_role = std::make_shared<CreateRole>("user", role_perm_group);

      auto create_domain = std::make_shared<CreateDomain>();
      create_domain->domain_id = "test";
      create_domain->user_default_role = "user";

      auto create_asset = std::make_shared<CreateAsset>();
      create_asset->domain_id = "test";
      create_asset->asset_name = "coin";
      create_asset->precision = 2;

      auto create_admin = std::make_shared<CreateAccount>();
      create_admin->domain_id = "test";
      create_admin->account_name = "admin";

      auto create_acc = std::make_shared<CreateAccount>();
      create_acc->domain_id = "test";
      create_acc->account_name = "test";

      transaction.commands = {
          create_domain, create_asset, create_admin, create_acc};
      return transaction;
    }

    Block generateBlock() {
      Block block;
      block.created_ts = 100500;
      block.height = 1;
      std::fill(block.prev_hash.begin(), block.prev_hash.end(), 0);
      block.txs_number = 4;

      auto start_port = 10001u;
      for (size_t i = start_port; i < start_port + block.txs_number; ++i) {
        block.transactions.push_back(getAddPeerTransaction(
            block.created_ts, "0.0.0.0:" + std::to_string(i)));
      }

      block.transactions.push_back(getTestCreateTransaction(block.created_ts));
      block.txs_number++;

      Signature sign{};
      block.sigs = {sign};
      block.hash = hash(block);
      return block;
    }
  }  // namespace generator
}  // namespace framework
