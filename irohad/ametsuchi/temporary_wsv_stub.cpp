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

#include <ametsuchi/temporary_wsv_stub.hpp>
#include <functional>

namespace iroha {

  namespace ametsuchi {

    bool TemporaryWsvStub::apply(
        const dao::Transaction &transaction,
        std::function<bool(const dao::Transaction &, CommandExecutor &,
                           WsvQuery &)>
            function) {
      return function(transaction, executor_, ametsuchi_);
    }

    dao::Account TemporaryWsvStub::get_account(ed25519::pubkey_t pub_key) {
      return ametsuchi_.get_account(pub_key);
    }

    dao::Asset TemporaryWsvStub::get_asset(std::string asset_full_name) {
      return ametsuchi_.get_asset(asset_full_name);
    }

    dao::Domain TemporaryWsvStub::get_domain(std::string domain_full_name) {
      return ametsuchi_.get_domain(domain_full_name);
    }

    dao::Wallet TemporaryWsvStub::get_wallet(std::string wallet_id) {
      return ametsuchi_.get_wallet(wallet_id);
    }

    std::vector<dao::Wallet> TemporaryWsvStub::get_account_wallets(
        ed25519::pubkey_t pub_key) {
      return ametsuchi_.get_account_wallets(pub_key);
    }

    std::vector<dao::Asset> TemporaryWsvStub::get_domain_assets(
        std::string domain_full_name) {
      return ametsuchi_.get_domain_assets(domain_full_name);
    }

    TemporaryWsvStub::TemporaryWsvStub(AmetsuchiStub &ametsuchi)
        : ametsuchi_(ametsuchi), executor_(*this) {}

    iroha::dao::Peer TemporaryWsvStub::get_peer(
        iroha::ed25519::pubkey_t pub_key) {
      return ametsuchi_.get_peer(pub_key);
    }
  }

}  // namespace iroha