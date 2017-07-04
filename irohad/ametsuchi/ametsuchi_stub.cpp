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

#include <ametsuchi/ametsuchi_stub.hpp>
#include <ametsuchi/mutable_storage_stub.hpp>
#include <ametsuchi/temporary_wsv_stub.hpp>
#include <iostream>

namespace iroha {
  namespace ametsuchi {

    std::unique_ptr<TemporaryWsv> AmetsuchiStub::createTemporaryWsv() {
      std::cout << "[Ametsuchi] creating temporary wsv" << std::endl;
      return std::make_unique<TemporaryWsvStub>(*this);
    }

    std::unique_ptr<MutableStorage> AmetsuchiStub::createMutableStorage() {
      std::cout << "[Ametsuchi] creating mutable storage" << std::endl;
      return std::make_unique<MutableStorageStub>(*this);
    }

    void AmetsuchiStub::commit(MutableStorage &mutableStorage) {
      std::cout << "[Ametsuchi] committing mutable storage" << std::endl;
      return;
    }

    rxcpp::observable<dao::Transaction> AmetsuchiStub::get_account_transactions(
        ed25519::pubkey_t pub_key) {
      return rxcpp::observable<>::create<dao::Transaction>(
          [](rxcpp::subscriber<dao::Transaction> s) {
            s.on_next(dao::Transaction{});
            s.on_completed();
          });
    }

    rxcpp::observable<dao::Transaction> AmetsuchiStub::get_asset_transactions(
        std::string asset_full_name) {
      return rxcpp::observable<>::create<dao::Transaction>(
          [](rxcpp::subscriber<dao::Transaction> s) {
            s.on_next(dao::Transaction{});
            s.on_completed();
          });
    }

    rxcpp::observable<dao::Transaction> AmetsuchiStub::get_wallet_transactions(
        std::string wallet_id) {
      return rxcpp::observable<>::create<dao::Transaction>(
          [](rxcpp::subscriber<dao::Transaction> s) {
            s.on_next(dao::Transaction{});
            s.on_completed();
          });
    }
    rxcpp::observable<dao::Block> AmetsuchiStub::get_blocks_in_range(
        uint32_t from, uint32_t to) {
      std::cout << "[Ametsuchi] get_blocks(" << from << ", " << to << ")"
                << std::endl;
      return rxcpp::observable<>::create<dao::Block>(
          [](rxcpp::subscriber<dao::Block> s) {
            s.on_next(dao::Block{});
            s.on_completed();
          });
    }
    dao::Account AmetsuchiStub::get_account(ed25519::pubkey_t pub_key) {
      return dao::Account{};
    }

    dao::Asset AmetsuchiStub::get_asset(std::string asset_full_name) {
      return dao::Asset{};
    }

    dao::Domain AmetsuchiStub::get_domain(std::string domain_full_name) {
      return dao::Domain{};
    }

    dao::Wallet AmetsuchiStub::get_wallet(std::string wallet_id) {
      return dao::Wallet{};
    }

    std::vector<dao::Wallet> AmetsuchiStub::get_account_wallets(
        ed25519::pubkey_t pub_key) {
      return std::vector<dao::Wallet>{dao::Wallet{}};
    }

    std::vector<dao::Asset> AmetsuchiStub::get_domain_assets(
        std::string domain_full_name) {
      return std::vector<dao::Asset>{dao::Asset{}};
    }

    dao::Peer AmetsuchiStub::get_peer(
        iroha::ed25519::pubkey_t pub_key) {
      return dao::Peer{};
    }

    AmetsuchiStub::~AmetsuchiStub() {
    }

  }  // namespace ametsuchi
}  // namespace iroha