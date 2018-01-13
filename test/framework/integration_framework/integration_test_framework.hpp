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
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "logger/logger.hpp"
#include "model/block.hpp"
#include "model/generators/command_generator.hpp"
#include "model/proposal.hpp"
#include "model/transaction.hpp"

namespace integration_framework {

  using std::chrono::milliseconds;

  class IntegrationTestFramework {
   private:
    using ProposalType = std::shared_ptr<iroha::model::Proposal>;
    using BlockType = std::shared_ptr<iroha::model::Block>;

   public:
    IntegrationTestFramework &setInitialState(
        const iroha::keypair_t &keypair = iroha::create_keypair());
    IntegrationTestFramework &setInitialState(const iroha::keypair_t &keypair,
                                              const iroha::model::Block &block);

    IntegrationTestFramework &addUser(
        std::string account_id,
        const iroha::keypair_t &keypair,
        std::vector<iroha::pubkey_t> signatories = {});

    template <typename Lambda>
    IntegrationTestFramework &sendTx(iroha::model::Transaction tx,
                                     Lambda validation);
    IntegrationTestFramework &sendTx(iroha::model::Transaction tx);

    template <typename Lambda>
    IntegrationTestFramework &sendQuery(const iroha::model::Query &qry,
                                        Lambda validation);
    IntegrationTestFramework &sendQuery(const iroha::model::Query &qry);

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
    static void fetchFromQueue(Queue &queue,
                               ObjectType &ref_for_insertion,
                               const WaitTime &wait,
                               const std::string &error_reason);

   protected:
    std::shared_ptr<IrohaInstance> iroha_instance_ =
        std::make_shared<IrohaInstance>();
    tbb::concurrent_queue<ProposalType> proposal_queue_;
    tbb::concurrent_queue<BlockType> block_queue_;

    // config area

    /// maximum time of waiting before appearing next proposal
    // TODO 21/12/2017 muratovv make relation of time with instance's config
    const milliseconds proposal_waiting = milliseconds(5000);

    /// maximum time of waiting before appearing next committed block
    const milliseconds block_waiting = milliseconds(5000);

    const std::string default_domain = "test";
    const std::string default_role = "user";

   private:
    logger::Logger log_ = logger::log("IntegrationTestFramework");
  };

  template <typename Lambda>
  IntegrationTestFramework &IntegrationTestFramework::sendTx(
      iroha::model::Transaction tx, Lambda validation) {
    log_->info("send transaction");
    // deserialize
    auto pb_tx = iroha::model::converters::PbTransactionFactory().serialize(tx);
    // send
    {
      google::protobuf::Empty response;
      iroha_instance_->getIrohaInstance()->getCommandService()->ToriiAsync(
          pb_tx, response);
    }
    // fetch status of transaction
    iroha::protocol::TxStatus status;
    {
      iroha::protocol::TxStatusRequest request;
      request.set_tx_hash(iroha::hash(tx).to_string());
      iroha::protocol::ToriiResponse response;
      iroha_instance_->getIrohaInstance()->getCommandService()->StatusAsync(
          request, response);
      status = response.tx_status();
    }
    // check validation function
    validation(status);
    return *this;
  }

  template <typename Lambda>
  IntegrationTestFramework &IntegrationTestFramework::sendQuery(
      const iroha::model::Query &qry, Lambda validation) {
    using iroha::operator|;
    log_->info("send query");
    // serialize without calling destructor on passed reference
    auto pb_qry = iroha::model::converters::PbQueryFactory().serialize(
        std::shared_ptr<const iroha::model::Query>(&qry, [](auto) {}));
    // send
    iroha::protocol::QueryResponse pb_response;
    iroha_instance_->getIrohaInstance()->getQueryService()->FindAsync(
        *pb_qry, pb_response);
    // deserialize
    auto response =
        iroha::model::converters::PbQueryResponseFactory().deserialize(
            pb_response);
    // check validation function
    validation(**response);
    return *this;
  }

  template <typename Lambda>
  IntegrationTestFramework &IntegrationTestFramework::checkBlock(
      Lambda validation) {
    // fetch first from block queue
    log_->info("check block");
    BlockType block;
    fetchFromQueue(block_queue_, block, block_waiting, "missed block");
    validation(*block);
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
    return *this;
  }

  template <typename Queue, typename ObjectType, typename WaitTime>
  void IntegrationTestFramework::fetchFromQueue(
      Queue &queue,
      ObjectType &ref_for_insertion,
      const WaitTime &wait,
      const std::string &error_reason) {
    if (!queue.try_pop(ref_for_insertion)) {
      std::this_thread::sleep_for(wait);
    }
    if (!queue.try_pop(ref_for_insertion)) {
      throw std::runtime_error(error_reason);
    }
  }
}  // namespace integration_framework

#endif  // IROHA_INTEGRATION_FRAMEWORK_HPP
