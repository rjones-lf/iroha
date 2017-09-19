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

#ifndef IROHA_AMETSUCHI_MOCKS_HPP
#define IROHA_AMETSUCHI_MOCKS_HPP

#include <gmock/gmock.h>
#include "ametsuchi/block_query.hpp"
#include "ametsuchi/mutable_factory.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "ametsuchi/peer_query.hpp"
#include "ametsuchi/temporary_factory.hpp"
#include "ametsuchi/temporary_wsv.hpp"
#include "ametsuchi/wsv_query.hpp"

namespace iroha {
  namespace ametsuchi {
    class MockWsvQuery : public WsvQuery {
     public:
      MOCK_METHOD1(getAccount, nonstd::optional<model::Account>(
                                   const std::string &account_id));
      MOCK_METHOD1(getSignatories,
                   nonstd::optional<std::vector<ed25519::pubkey_t>>(
                       const std::string &account_id));
      MOCK_METHOD1(getAsset,
                   nonstd::optional<model::Asset>(const std::string &asset_id));
      MOCK_METHOD2(getAccountAsset, nonstd::optional<model::AccountAsset>(
                                        const std::string &account_id,
                                        const std::string &asset_id));
      MOCK_METHOD0(getPeers, nonstd::optional<std::vector<model::Peer>>());
    };

    class MockWsvCommand : public WsvCommand {
     public:
      MOCK_METHOD1(insertAccount, bool(const model::Account &));
      MOCK_METHOD1(updateAccount, bool(const model::Account &));
      MOCK_METHOD1(insertAsset, bool(const model::Asset &));
      MOCK_METHOD1(upsertAccountAsset, bool(const model::AccountAsset &));
      MOCK_METHOD1(insertSignatory, bool(const ed25519::pubkey_t &));

      MOCK_METHOD2(insertAccountSignatory,
                   bool(const std::string &, const ed25519::pubkey_t &));

      MOCK_METHOD2(deleteAccountSignatory,
                   bool(const std::string &, const ed25519::pubkey_t &));


      MOCK_METHOD1(insertPeer, bool(const model::Peer &));

      MOCK_METHOD1(deletePeer, bool(const model::Peer &));

      MOCK_METHOD1(insertDomain, bool(const model::Domain &));
    };

    class MockBlockQuery : public BlockQuery {
     public:
      MOCK_METHOD1(
          getAccountTransactions,
          rxcpp::observable<model::Transaction>(std::string account_id));
      MOCK_METHOD2(
          getAccountAssetTransactions,
          rxcpp::observable<model::Transaction>(std::string account_id, std::string asset_id));
      MOCK_METHOD2(getBlocks,
                   rxcpp::observable<model::Block>(uint32_t, uint32_t));
      MOCK_METHOD1(getBlocksFrom, rxcpp::observable<model::Block>(uint32_t));
      MOCK_METHOD1(getTopBlocks, rxcpp::observable<model::Block>(uint32_t));
    };

    class MockTemporaryFactory : public TemporaryFactory {
     public:
      MOCK_METHOD0(createTemporaryWsv, std::unique_ptr<TemporaryWsv>());
    };

    class MockMutableStorage : public MutableStorage {
     public:
      MOCK_METHOD2(apply,
                   bool(const model::Block &,
                        std::function<bool(const model::Block &, WsvQuery &,
                                           const hash256_t &)>));
    };

    /**
     * Factory for generation mock mutable storages.
     * This method provide technique,
     * when required to return object wrapped in unique pointer.
     */
    std::unique_ptr<MutableStorage> createMockMutableStorage() {
      return std::make_unique<MockMutableStorage>();
    }

    class MockMutableFactory : public MutableFactory {
     public:
      MOCK_METHOD0(createMutableStorage, std::unique_ptr<MutableStorage>());

      void commit(std::unique_ptr<MutableStorage> mutableStorage) override {
        // gmock workaround for non-copyable parameters
        commit_(mutableStorage);
      }

      MOCK_METHOD1(commit_, void(std::unique_ptr<MutableStorage> &));
    };

    class MockPeerQuery : public PeerQuery {
     public:
      MockPeerQuery() = default;

      MOCK_METHOD0(getLedgerPeers,
                   nonstd::optional<std::vector<model::Peer>>());
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_MOCKS_HPP
