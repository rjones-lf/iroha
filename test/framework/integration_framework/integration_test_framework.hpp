/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_INTEGRATION_FRAMEWORK_HPP
#define IROHA_INTEGRATION_FRAMEWORK_HPP

#include <tbb/concurrent_queue.h>
#include <algorithm>
#include <chrono>
#include <exception>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include "crypto/keys_manager_impl.hpp"
#include "cryptography/blob.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "cryptography/keypair.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "logger/logger.hpp"

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/proposal.hpp"
#include "backend/protobuf/queries/proto_query.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "backend/protobuf/block.hpp"
#include "backend/protobuf/proposal.hpp"

namespace integration_framework {

  using std::chrono::milliseconds;

  class IntegrationTestFramework {
   private:
    using ProposalType = std::shared_ptr<shared_model::interface::Proposal>;
    using BlockType = std::shared_ptr<shared_model::interface::Block>;

   public:
    IntegrationTestFramework(size_t maximum_block_size = 10)
        : maximum_block_size_(maximum_block_size) {}
    ~IntegrationTestFramework();
    IntegrationTestFramework &setInitialState(
        const shared_model::crypto::Keypair &keypair);
    IntegrationTestFramework &setInitialState(
        const shared_model::crypto::Keypair &keypair,
        const shared_model::interface::Block &block);

    template <typename Lambda>
    IntegrationTestFramework &sendTx(const shared_model::proto::Transaction &tx,
                                     Lambda validation);
    IntegrationTestFramework &sendTx(
        const shared_model::proto::Transaction &tx);
    shared_model::proto::TransactionResponse getTxStatus(
        const shared_model::crypto::Hash &hash);

    template <typename Lambda>
    IntegrationTestFramework &sendQuery(const shared_model::proto::Query &qry,
                                        Lambda validation);
    IntegrationTestFramework &sendQuery(const shared_model::proto::Query &qry);

    template <typename Lambda>
    IntegrationTestFramework &checkProposal(Lambda validation);
    IntegrationTestFramework &skipProposal();

    template <typename Lambda>
    IntegrationTestFramework &checkBlock(Lambda validation);
    IntegrationTestFramework &skipBlock();

    /**
     * Shutdown iroha
     */
    void done();

    /**
     * general way to fetch object from concurrent queue
     * @tparam Queue - Type of queue
     * @tparam ObjectType - Type of fetched object
     * @tparam WaitTime - time for waiting if data doesn't appear
     * @param queue - queue instance for fetching
     * @param ref_for_insertion - reference to insert object
     * @param wait - time of waiting
     * @param error_reason - reason if thehre is no appeared object at all
     */
    template <typename Queue, typename ObjectType, typename WaitTime>
    void fetchFromQueue(Queue &queue,
                        ObjectType &ref_for_insertion,
                        const WaitTime &wait,
                        const std::string &error_reason);

    static const std::string kDefaultDomain;
    static const std::string kDefaultRole;

    static const std::string kAdminName;
    static const std::string kAdminId;
    static const std::string kAssetName;

   protected:
    tbb::concurrent_queue<ProposalType> proposal_queue_;
    tbb::concurrent_queue<BlockType> block_queue_;
    std::shared_ptr<IrohaInstance> iroha_instance_ =
        std::make_shared<IrohaInstance>();

    // config area

    /// maximum time of waiting before appearing next proposal
    // TODO 21/12/2017 muratovv make relation of time with instance's config
    const milliseconds proposal_waiting = milliseconds(20000);

    /// maximum time of waiting before appearing next committed block
    const milliseconds block_waiting = milliseconds(20000);

    size_t maximum_block_size_;

   private:
    logger::Logger log_ = logger::log("IntegrationTestFramework");
    std::mutex queue_mu;
    std::condition_variable queue_cond;
  };

  template <typename Lambda>
  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      const shared_model::proto::Transaction &tx, Lambda validation) {
    log_->info("send transaction");
    iroha_instance_->getIrohaInstance()->getCommandService()->Torii(
        tx.getTransport());
    // fetch status of transaction
    shared_model::proto::TransactionResponse status = getTxStatus(tx.hash());

    // check validation function
    validation(status);
    return *this;
  }

  template <typename Lambda>
  IntegrationTestFramework &IntegrationTestFramework::sendQuery(
      const shared_model::proto::Query &qry, Lambda validation) {
    log_->info("send query");

    iroha::protocol::QueryResponse response;
    iroha_instance_->getIrohaInstance()->getQueryService()->Find(
        qry.getTransport(), response);
    auto query_response =
        shared_model::proto::QueryResponse(std::move(response));

    validation(query_response);
    return *this;
  }

  template <typename Lambda>
  IntegrationTestFramework &IntegrationTestFramework::checkBlock(
      Lambda validation) {
    // fetch first from block queue
    log_->info("check block");
    BlockType block;
    fetchFromQueue(block_queue_, block, block_waiting, "missed block");
    validation(block);
    return *this;
  }

  template <typename Lambda>
  IntegrationTestFramework &IntegrationTestFramework::checkProposal(
      Lambda validation) {
    log_->info("check proposal");
    // fetch first proposal from proposal queue
    ProposalType proposal;
    fetchFromQueue(
        proposal_queue_, proposal, proposal_waiting, "missed proposal");
    validation(proposal);
    return *this;
  }

  template <typename Queue, typename ObjectType, typename WaitTime>
  void IntegrationTestFramework::fetchFromQueue(
      Queue &queue,
      ObjectType &ref_for_insertion,
      const WaitTime &wait,
      const std::string &error_reason) {
    std::unique_lock<std::mutex> lk(queue_mu);
    queue_cond.wait_for(lk, wait, [&]() { return not queue.empty(); });
    if (!queue.try_pop(ref_for_insertion)) {
      throw std::runtime_error(error_reason);
    }
  }
}  // namespace integration_framework

#endif  // IROHA_INTEGRATION_FRAMEWORK_HPP
