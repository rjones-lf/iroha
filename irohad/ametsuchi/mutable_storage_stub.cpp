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

#include <ametsuchi/mutable_storage_stub.hpp>

namespace iroha {
  namespace ametsuchi {

    rxcpp::observable<model::Transaction>
    MutableStorageStub::get_account_transactions(ed25519::pubkey_t pub_key) {
      return ametsuchi_.get_account_transactions(pub_key);
    }

    rxcpp::observable<model::Transaction>
    MutableStorageStub::get_asset_transactions(std::string asset_full_name) {
      return ametsuchi_.get_asset_transactions(asset_full_name);
    }

    rxcpp::observable<model::Transaction>
    MutableStorageStub::get_wallet_transactions(std::string wallet_id) {
      return ametsuchi_.get_wallet_transactions(wallet_id);
    }

    model::Account MutableStorageStub::get_account(ed25519::pubkey_t pub_key) {
      return ametsuchi_.get_account(pub_key);
    }

    model::Asset MutableStorageStub::get_asset(std::string asset_full_name) {
      return ametsuchi_.get_asset(asset_full_name);
    }

    model::Domain MutableStorageStub::get_domain(std::string domain_full_name) {
      return ametsuchi_.get_domain(domain_full_name);
    }

    model::Peer MutableStorageStub::get_peer(
        ed25519::pubkey_t pub_key) {
      return ametsuchi_.get_peer(pub_key);
    }

    model::Wallet MutableStorageStub::get_wallet(std::string wallet_id) {
      return ametsuchi_.get_wallet(wallet_id);
    }

    std::vector<model::Wallet> MutableStorageStub::get_account_wallets(
        ed25519::pubkey_t pub_key) {
      return ametsuchi_.get_account_wallets(pub_key);
    }

    std::vector<model::Asset> MutableStorageStub::get_domain_assets(
        std::string domain_full_name) {
      return ametsuchi_.get_domain_assets(domain_full_name);
    }

    bool MutableStorageStub::apply(
        const model::Block &block,
        std::function<bool(const model::Block &, CommandExecutor &, WsvQuery &)>
            function) {
      return function(block, executor_, ametsuchi_);
    }

    MutableStorageStub::MutableStorageStub(AmetsuchiStub &ametsuchi)
        : ametsuchi_(ametsuchi), executor_(*this) {}

    rxcpp::observable<model::Block> MutableStorageStub::get_blocks_in_range(
        uint32_t from, uint32_t to) {
      return ametsuchi_.get_blocks_in_range(from, to);
    }
  }  // namespace ametsuchi
}  // namespace iroha
