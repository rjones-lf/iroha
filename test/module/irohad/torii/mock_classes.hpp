/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef IROHA_MOCK_CLASSES_HPP
#define IROHA_MOCK_CLASSES_HPP
#include <gmock/gmock.h>
#include "ametsuchi/block_query.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "model/query_execution.hpp"
#include "network/peer_communication_service.hpp"
#include "validation/stateless_validator.hpp"

class PCSMock : public iroha::network::PeerCommunicationService {
 public:
  MOCK_METHOD1(propagate_transaction, void(iroha::model::Transaction));

  MOCK_METHOD0(on_proposal, rxcpp::observable<iroha::model::Proposal>());

  MOCK_METHOD0(on_commit, rxcpp::observable<iroha::network::Commit>());
};

class StatelessValidatorMock : public iroha::validation::StatelessValidator {
 public:
  MOCK_CONST_METHOD1(validate, bool(const iroha::model::Transaction &));
  MOCK_CONST_METHOD1(validate, bool(const iroha::model::Query &));
};

/**
 * Mock for wsv query
 */
class WsvQueryMock : public iroha::ametsuchi::WsvQuery {
 public:
  MOCK_METHOD1(getAccount, nonstd::optional<iroha::model::Account>(
                               const std::string &account_id));
  MOCK_METHOD1(getSignatories,
               nonstd::optional<std::vector<iroha::ed25519::pubkey_t>>(
                   const std::string &account_id));
  MOCK_METHOD1(getAsset, nonstd::optional<iroha::model::Asset>(
                             const std::string &asset_id));
  MOCK_METHOD2(getAccountAsset,
               nonstd::optional<iroha::model::AccountAsset>(
                   const std::string &account_id, const std::string &asset_id));
  MOCK_METHOD0(getPeers, nonstd::optional<std::vector<iroha::model::Peer>>());
};

/**
 * Mock for block query
 */
class BlockQueryMock : public iroha::ametsuchi::BlockQuery {
 public:
  MOCK_METHOD1(
      getAccountTransactions,
      rxcpp::observable<iroha::model::Transaction>(std::string account_id));
  MOCK_METHOD2(getBlocks, rxcpp::observable<iroha::model::Block>(uint32_t from,
                                                                 uint32_t to));
};

/**
 * Mock for query processing factory
 */
class QpfMock : public iroha::model::QueryProcessingFactory {
 public:
  MOCK_METHOD1(execute, std::shared_ptr<iroha::model::QueryResponse>(
                            const iroha::model::Query &query));
};

#endif  // IROHA_MOCK_CLASSES_HPP
