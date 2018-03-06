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
#include <boost/optional.hpp>
#include "ametsuchi/block_query.hpp"
#include "ametsuchi/mutable_factory.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "ametsuchi/peer_query.hpp"
#include "ametsuchi/storage.hpp"
#include "ametsuchi/temporary_factory.hpp"
#include "ametsuchi/temporary_wsv.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "common/result.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "model/account.hpp"
#include "model/account_asset.hpp"
#include "model/asset.hpp"
#include "model/block.hpp"
#include "model/domain.hpp"
#include "model/peer.hpp"
#include "model/transaction.hpp"

namespace iroha {
  namespace ametsuchi {
    class MockWsvQuery : public WsvQuery {
     public:
      MOCK_METHOD1(getAccountRoles,
                   nonstd::optional<std::vector<std::string>>(
                       const std::string &account_id));
      MOCK_METHOD3(
          getAccountDetail,
          nonstd::optional<std::string>(const std::string &account_id,
                                        const std::string &creator_account_id,
                                        const std::string &detail));
      MOCK_METHOD1(getRolePermissions,
                   nonstd::optional<std::vector<std::string>>(
                       const std::string &role_name));
      MOCK_METHOD0(getRoles, nonstd::optional<std::vector<std::string>>());
      MOCK_METHOD1(
          getAccount,
          nonstd::optional<std::shared_ptr<shared_model::interface::Account>>(
              const std::string &account_id));
      MOCK_METHOD1(getSignatories,
                   nonstd::optional<
                       std::vector<shared_model::interface::types::PubkeyType>>(
                       const std::string &account_id));
      MOCK_METHOD1(
          getAsset,
          nonstd::optional<std::shared_ptr<shared_model::interface::Asset>>(
              const std::string &asset_id));
      MOCK_METHOD2(getAccountAsset,
                   nonstd::optional<
                       std::shared_ptr<shared_model::interface::AccountAsset>>(
                       const std::string &account_id,
                       const std::string &asset_id));
      MOCK_METHOD0(
          getPeers,
          nonstd::optional<
              std::vector<std::shared_ptr<shared_model::interface::Peer>>>());
      MOCK_METHOD1(
          getDomain,
          nonstd::optional<std::shared_ptr<shared_model::interface::Domain>>(
              const std::string &domain_id));
      MOCK_METHOD3(hasAccountGrantablePermission,
                   bool(const std::string &permitee_account_id,
                        const std::string &account_id,
                        const std::string &permission_id));
    };

    class MockWsvCommand : public WsvCommand {
     public:
      MOCK_METHOD1(insertRole, WsvCommandResult(const std::string &role_name));
      MOCK_METHOD2(insertAccountRole,
                   WsvCommandResult(const std::string &account_id,
                                    const std::string &role_name));
      MOCK_METHOD2(deleteAccountRole,
                   WsvCommandResult(const std::string &account_id,
                                    const std::string &role_name));
      MOCK_METHOD2(insertRolePermissions,
                   WsvCommandResult(const std::string &role_id,
                                    const std::set<std::string> &permissions));

      MOCK_METHOD3(insertAccountGrantablePermission,
                   WsvCommandResult(const std::string &permittee_account_id,
                                    const std::string &account_id,
                                    const std::string &permission_id));

      MOCK_METHOD3(deleteAccountGrantablePermission,
                   WsvCommandResult(const std::string &permittee_account_id,
                                    const std::string &account_id,
                                    const std::string &permission_id));
      MOCK_METHOD1(insertAccount, WsvCommandResult(const model::Account &));
      MOCK_METHOD1(updateAccount, WsvCommandResult(const model::Account &));
      MOCK_METHOD1(insertAsset, WsvCommandResult(const model::Asset &));
      MOCK_METHOD1(upsertAccountAsset,
                   WsvCommandResult(const model::AccountAsset &));
      MOCK_METHOD1(insertSignatory, WsvCommandResult(const pubkey_t &));
      MOCK_METHOD1(deleteSignatory, WsvCommandResult(const pubkey_t &));

      MOCK_METHOD2(insertAccountSignatory,
                   WsvCommandResult(const std::string &, const pubkey_t &));

      MOCK_METHOD2(deleteAccountSignatory,
                   WsvCommandResult(const std::string &, const pubkey_t &));

      MOCK_METHOD1(insertPeer, WsvCommandResult(const model::Peer &));

      MOCK_METHOD1(deletePeer, WsvCommandResult(const model::Peer &));

      MOCK_METHOD1(insertDomain, WsvCommandResult(const model::Domain &));
      MOCK_METHOD4(setAccountKV,
                   WsvCommandResult(const std::string &,
                                    const std::string &,
                                    const std::string &,
                                    const std::string &));
    };

    class MockBlockQuery : public BlockQuery {
     public:
      MOCK_METHOD1(
          getAccountTransactions,
          rxcpp::observable<wTransaction>(
              const shared_model::interface::types::AccountIdType &account_id));
      MOCK_METHOD1(getTxByHashSync,
                   boost::optional<wTransaction>(
                       const shared_model::crypto::Hash &hash));
      MOCK_METHOD2(
          getAccountAssetTransactions,
          rxcpp::observable<wTransaction>(
              const shared_model::interface::types::AccountIdType &account_id,
              const shared_model::interface::types::AssetIdType &asset_id));
      MOCK_METHOD1(
          getTransactions,
          rxcpp::observable<boost::optional<wTransaction>>(
              const std::vector<shared_model::crypto::Hash> &tx_hashes));
      MOCK_METHOD2(getBlocks,
                   rxcpp::observable<wBlock>(
                       shared_model::interface::types::HeightType, uint32_t));
      MOCK_METHOD1(getBlocksFrom,
                   rxcpp::observable<wBlock>(
                       shared_model::interface::types::HeightType));
      MOCK_METHOD1(getTopBlocks, rxcpp::observable<wBlock>(uint32_t));
    };

    class MockTemporaryFactory : public TemporaryFactory {
     public:
      MOCK_METHOD0(
          createTemporaryWsv,
          expected::Result<std::unique_ptr<TemporaryWsv>, std::string>(void));
    };

    class MockMutableStorage : public MutableStorage {
     public:
      MOCK_METHOD2(
          apply,
          bool(const shared_model::interface::Block &,
               std::function<
                   bool(const shared_model::interface::Block &,
                        WsvQuery &,
                        const shared_model::interface::types::HashType &)>));
    };

    /**
     * Factory for generation mock mutable storages.
     * This method provide technique,
     * when required to return object wrapped in Result.
     */
    expected::Result<std::unique_ptr<MutableStorage>, std::string>
    createMockMutableStorage() {
      return expected::makeValue<std::unique_ptr<MutableStorage>>(
          std::make_unique<MockMutableStorage>());
    }

    class MockMutableFactory : public MutableFactory {
     public:
      MOCK_METHOD0(
          createMutableStorage,
          expected::Result<std::unique_ptr<MutableStorage>, std::string>(void));

      void commit(std::unique_ptr<MutableStorage> mutableStorage) override {
        // gmock workaround for non-copyable parameters
        commit_(mutableStorage);
      }

      MOCK_METHOD1(commit_, void(std::unique_ptr<MutableStorage> &));
    };

    class MockPeerQuery : public PeerQuery {
     public:
      MockPeerQuery() = default;

      MOCK_METHOD0(getLedgerPeers, boost::optional<std::vector<wPeer>>());
    };

    class MockStorage : public Storage {
     public:
      MOCK_CONST_METHOD0(getWsvQuery, std::shared_ptr<WsvQuery>(void));
      MOCK_CONST_METHOD0(getBlockQuery, std::shared_ptr<BlockQuery>(void));
      MOCK_METHOD0(
          createTemporaryWsv,
          expected::Result<std::unique_ptr<TemporaryWsv>, std::string>(void));
      MOCK_METHOD0(
          createMutableStorage,
          expected::Result<std::unique_ptr<MutableStorage>, std::string>(void));
      MOCK_METHOD1(doCommit, void(MutableStorage *storage));
      MOCK_METHOD1(insertBlock, bool(model::Block block));
      MOCK_METHOD1(insertBlocks, bool(const std::vector<model::Block> &blocks));
      MOCK_METHOD0(dropStorage, void(void));

      void commit(std::unique_ptr<MutableStorage> storage) override {
        doCommit(storage.get());
      }
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_AMETSUCHI_MOCKS_HPP
