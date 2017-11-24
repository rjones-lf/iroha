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
#include "ametsuchi/storage.hpp"
#include "ametsuchi/temporary_factory.hpp"
#include "ametsuchi/temporary_wsv.hpp"
#include "ametsuchi/wsv_query.hpp"

#include <boost/optional.hpp>

namespace iroha {
  namespace ametsuchi {
    class MockWsvQuery : public WsvQuery {
     public:
      MOCK_METHOD1(getAccountRoles,
                   nonstd::optional<std::vector<std::string>>(
                       const std::string &account_id));
      MOCK_METHOD1(getRolePermissions,
                   nonstd::optional<std::vector<std::string>>(
                       const std::string &role_name));
      MOCK_METHOD0(getRoles, nonstd::optional<std::vector<std::string>>());
      MOCK_METHOD1(
          getAccount,
          nonstd::optional<model::Account>(const std::string &account_id));
      MOCK_METHOD1(getSignatories,
                   nonstd::optional<std::vector<pubkey_t>>(
                       const std::string &account_id));
      MOCK_METHOD1(getAsset,
                   nonstd::optional<model::Asset>(const std::string &asset_id));
      MOCK_METHOD2(
          getAccountAsset,
          nonstd::optional<model::AccountAsset>(const std::string &account_id,
                                                const std::string &asset_id));
      MOCK_METHOD0(getPeers, nonstd::optional<std::vector<model::Peer>>());
      MOCK_METHOD1(
          getDomain,
          nonstd::optional<model::Domain>(const std::string &domain_id));
      MOCK_METHOD3(hasAccountGrantablePermission,
                   bool(const std::string &permitee_account_id,
                        const std::string &account_id,
                        const std::string &permission_id));
    };

    class MockWsvCommand : public WsvCommand {
     public:
      MOCK_METHOD1(insertRole, bool(const std::string &role_name));
      MOCK_METHOD2(insertAccountRole,
                   bool(const std::string &account_id,
                        const std::string &role_name));
      MOCK_METHOD2(insertRolePermissions,
                   bool(const std::string &role_id,
                        const std::set<std::string> &permissions));

      MOCK_METHOD3(insertAccountGrantablePermission,
                   bool(const std::string &permittee_account_id,
                        const std::string &account_id,
                        const std::string &permission_id));

      MOCK_METHOD3(deleteAccountGrantablePermission,
                   bool(const std::string &permittee_account_id,
                        const std::string &account_id,
                        const std::string &permission_id));
      MOCK_METHOD1(insertAccount, bool(const model::Account &));
      MOCK_METHOD1(updateAccount, bool(const model::Account &));
      MOCK_METHOD1(insertAsset, bool(const model::Asset &));
      MOCK_METHOD1(upsertAccountAsset, bool(const model::AccountAsset &));
      MOCK_METHOD1(insertSignatory, bool(const pubkey_t &));
      MOCK_METHOD1(deleteSignatory, bool(const pubkey_t &));

      MOCK_METHOD2(insertAccountSignatory,
                   bool(const std::string &, const pubkey_t &));

      MOCK_METHOD2(deleteAccountSignatory,
                   bool(const std::string &, const pubkey_t &));

      MOCK_METHOD1(insertPeer, bool(const model::Peer &));

      MOCK_METHOD1(deletePeer, bool(const model::Peer &));

      MOCK_METHOD1(insertDomain, bool(const model::Domain &));
    };

    class MockBlockQuery : public BlockQuery {
     public:
      MOCK_METHOD2(getAccountTransactions,
          rxcpp::observable<model::Transaction>(const std::string&,
                                              const model::Pager&));
      MOCK_METHOD1(
          getTxByHashSync,
          boost::optional<model::Transaction>(const std::string &hash));
      MOCK_METHOD2(
          getAccountAssetTransactions,
          rxcpp::observable<model::Transaction>(const std::string &account_id,
                                                const std::string &asset_id));
      MOCK_METHOD1(getTransactions,
                   rxcpp::observable<boost::optional<model::Transaction>>(
                     const std::vector<iroha::hash256_t> &tx_hashes));
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
      MOCK_METHOD2(
          apply,
          bool(const model::Block &,
               std::function<bool(
                   const model::Block &, WsvQuery &, const hash256_t &)>));
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

    class MockStorage : public Storage {
     public:
      MOCK_CONST_METHOD0(getWsvQuery, std::shared_ptr<WsvQuery>(void));
      MOCK_CONST_METHOD0(getBlockQuery, std::shared_ptr<BlockQuery>(void));
      MOCK_METHOD0(createTemporaryWsv, std::unique_ptr<TemporaryWsv>(void));
      MOCK_METHOD0(createMutableStorage, std::unique_ptr<MutableStorage>(void));
      MOCK_METHOD1(doCommit, void(MutableStorage *storage));
      MOCK_METHOD1(insertBlock, bool(model::Block block));
      MOCK_METHOD0(dropStorage, void(void));

      void commit(std::unique_ptr<MutableStorage> storage) override {
        doCommit(storage.get());
      }
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_MOCKS_HPP
