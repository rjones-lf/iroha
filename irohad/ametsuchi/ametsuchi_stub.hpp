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

#ifndef IROHA_AMETSUCHI_STUB_HPP
#define IROHA_AMETSUCHI_STUB_HPP

#include <ametsuchi/ametsuchi.hpp>
#include <memory>

namespace iroha {
  namespace ametsuchi {

    class AmetsuchiStub : public Ametsuchi {
     public:
      model::Peer get_peer(
          iroha::ed25519::pubkey_t pub_key) override;
      std::unique_ptr<TemporaryWsv> createTemporaryWsv() override;
      std::unique_ptr<MutableStorage> createMutableStorage() override;
      void commit(MutableStorage &mutableStorage) override;
      rxcpp::observable<model::Transaction> get_account_transactions(
          ed25519::pubkey_t pub_key) override;
      rxcpp::observable<model::Transaction> get_asset_transactions(
          std::string asset_full_name) override;
      rxcpp::observable<model::Transaction> get_wallet_transactions(
          std::string wallet_id) override;
      rxcpp::observable<iroha::model::Block> get_blocks_in_range(
          uint32_t from, uint32_t to) override;
      model::Account get_account(ed25519::pubkey_t pub_key) override;
      model::Asset get_asset(std::string asset_full_name) override;
      model::Domain get_domain(std::string domain_full_name) override;
      model::Wallet get_wallet(std::string wallet_id) override;
      std::vector<model::Wallet> get_account_wallets(
          ed25519::pubkey_t pub_key) override;
      std::vector<model::Asset> get_domain_assets(
          std::string domain_full_name) override;
      ~AmetsuchiStub() override;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_STUB_HPP
