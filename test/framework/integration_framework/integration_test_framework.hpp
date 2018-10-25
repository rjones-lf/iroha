/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_INTEGRATION_FRAMEWORK_HPP
#define IROHA_INTEGRATION_FRAMEWORK_HPP

#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <tbb/concurrent_queue.h>
#include <boost/filesystem.hpp>
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"
#include "framework/integration_framework/iroha_instance.hpp"
#include "framework/integration_framework/test_irohad.hpp"
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "logger/logger.hpp"
#include "torii/command_client.hpp"
#include "torii/query_client.hpp"

namespace shared_model {
  namespace crypto {
    class Keypair;
  }
  namespace interface {
    class Block;
    class Proposal;
  }  // namespace interface
  namespace proto {
    class Block;
  }
}  // namespace shared_model

namespace integration_framework {

  using std::chrono::milliseconds;

  class IntegrationTestFramework {
   private:
    using ProposalType = std::shared_ptr<shared_model::interface::Proposal>;
    using BlockType = std::shared_ptr<shared_model::interface::Block>;
    using TxResponseType =
        std::shared_ptr<shared_model::interface::TransactionResponse>;

   public:
    /**
     * Construct test framework instance
     * @param maximum_proposal_size - Maximum number of transactions per
     * proposal
     * @param proposal_waiting - maximum time of waiting before next proposal
     * @param block_waiting - maximum time of waiting before appearing next
     * committed block
     * @param destructor_lambda - (default nullptr) Pointer to function which
     * receives pointer to constructed instance of Integration Test Framework.
     * If specified, then will be called instead of default destructor's code
     * @param mst_support enables multisignature tx support
     * @param block_store_path specifies path where blocks will be stored
     */
    explicit IntegrationTestFramework(
        size_t maximum_proposal_size,
        const boost::optional<std::string> &dbname = boost::none,
        std::function<void(IntegrationTestFramework &)> deleter =
            [](IntegrationTestFramework &itf) { itf.done(); },
        bool mst_support = false,
        const std::string &block_store_path =
            (boost::filesystem::temp_directory_path()
             / boost::filesystem::unique_path())
                .string(),
        milliseconds proposal_waiting = milliseconds(20000),
        milliseconds block_waiting = milliseconds(20000),
        milliseconds tx_response_waiting = milliseconds(10000));

    ~IntegrationTestFramework();

    /**
     * Construct default genesis block.
     *
     * Genesis block contains single transaction that
     * creates a single role (kDefaultRole), domain (kDefaultDomain),
     * account (kAdminName) and asset (kAssetName).
     * @param key - signing key
     * @return signed genesis block
     */
    static shared_model::proto::Block defaultBlock(
        const shared_model::crypto::Keypair &key);

    /**
     * Initialize Iroha instance with default genesis block and provided signing
     * key
     * @param keypair - signing key
     * @return this
     */
    IntegrationTestFramework &setInitialState(
        const shared_model::crypto::Keypair &keypair);

    /**
     * Initialize Iroha instance with provided genesis block and signing key
     * @param keypair - signing key
     * @param block - genesis block used for iroha initialization
     * @return this
     */
    IntegrationTestFramework &setInitialState(
        const shared_model::crypto::Keypair &keypair,
        const shared_model::interface::Block &block);

    /**
     * Initialize Iroha instance using the data left in block store from
     * previous launch of Iroha
     * @param keypair - signing key used for initialization of previous instance
     */
    IntegrationTestFramework &recoverState(
        const shared_model::crypto::Keypair &keypair);

    /**
     * Send transaction to Iroha and validate its status
     * @param tx - transaction for sending
     * @param validation - callback for transaction status validation that
     * receives object of type \relates shared_model::proto::TransactionResponse
     * by reference
     * @return this
     */
    IntegrationTestFramework &sendTx(
        const shared_model::proto::Transaction &tx,
        std::function<void(const shared_model::proto::TransactionResponse &)>
            validation);

    /**
     * Send transaction to Iroha without status validation
     * @param tx - transaction for sending
     * @return this
     */
    IntegrationTestFramework &sendTx(
        const shared_model::proto::Transaction &tx);

    /**
     * Send transaction to Iroha with awaiting proposal
     * and without status validation
     * @param tx - transaction for sending
     * @param check - callback for checking committed block
     * @return this
     */
    IntegrationTestFramework &sendTxAwait(
        const shared_model::proto::Transaction &tx,
        std::function<void(const BlockType &)> check);

    /**
     * Send transactions to Iroha and validate obtained statuses
     * @param tx_sequence - transactions sequence
     * @param validation - callback for transactions statuses validation.
     * Applied to the vector of returned statuses
     * @return this
     */
    IntegrationTestFramework &sendTxSequence(
        const shared_model::interface::TransactionSequence &tx_sequence,
        std::function<void(std::vector<shared_model::proto::TransactionResponse>
                               &)> validation = [](const auto &) {});

    /**
     * Send transactions to Iroha with awaiting proposal and without status
     * validation
     * @param tx_sequence - sequence for sending
     * @param check - callback for checking committed block
     * @return this
     */
    IntegrationTestFramework &sendTxSequenceAwait(
        const shared_model::interface::TransactionSequence &tx_sequence,
        std::function<void(const BlockType &)> check);

    /**
     * Check current status of transaction
     * @param hash - hash of transaction to check
     * @param validation - callback that receives transaction response
     * @return this
     */
    IntegrationTestFramework &getTxStatus(
        const shared_model::crypto::Hash &hash,
        std::function<void(const shared_model::proto::TransactionResponse &)>
            validation);

    /**
     * Send query to Iroha and validate the response
     * @param qry - query to be requested
     * @param validation - callback for query result check that receives object
     * of type \relates shared_model::proto::QueryResponse by reference
     * @return this
     */
    IntegrationTestFramework &sendQuery(
        const shared_model::proto::Query &qry,
        std::function<void(const shared_model::proto::QueryResponse &)>
            validation);

    /**
     * Send query to Iroha without response validation
     * @param qry - query to be requested
     * @return this
     */
    IntegrationTestFramework &sendQuery(const shared_model::proto::Query &qry);

    /**
     * Request next proposal from queue and serve it with custom handler
     * @param validation - callback that receives object of type \relates
     * std::shared_ptr<shared_model::interface::Proposal> by reference
     * @return this
     */
    IntegrationTestFramework &checkProposal(
        std::function<void(const ProposalType &)> validation);

    /**
     * Request next proposal from queue and skip it
     * @return this
     */
    IntegrationTestFramework &skipProposal();

    /**
     * Request next verified proposal from queue and check it with provided
     * function
     * @param validation - callback that receives object of type \relates
     * std::shared_ptr<shared_model::interface::Proposal> by reference
     * @return this
     */
    IntegrationTestFramework &checkVerifiedProposal(
        std::function<void(const ProposalType &)> validation);

    /**
     * Request next verified proposal from queue and skip it
     * @return this
     */
    IntegrationTestFramework &skipVerifiedProposal();

    /**
     * Request next block from queue and serve it with custom handler
     * @param validation - callback that receives object of type \relates
     * std::shared_ptr<shared_model::interface::Block> by reference
     * @return this
     */
    IntegrationTestFramework &checkBlock(
        std::function<void(const BlockType &)> validation);

    /**
     * Request next block from queue and skip it
     * @return this
     */
    IntegrationTestFramework &skipBlock();

    /**
     * Request next status of the transaction
     * @param tx_hash is hash for filtering responses
     * @return this
     */
    IntegrationTestFramework &checkStatus(
        const shared_model::interface::types::HashType &tx_hash,
        std::function<void(const shared_model::proto::TransactionResponse &)>
            validation);

    /**
     * Shutdown ITF instance
     */
    void done();

    static const std::string kDefaultDomain;
    static const std::string kDefaultRole;

    static const std::string kAdminName;
    static const std::string kAdminId;
    static const std::string kAssetName;

   protected:
    /**
     * general way to fetch object from concurrent queue
     * @tparam Queue - Type of queue
     * @tparam ObjectType - Type of fetched object
     * @tparam WaitTime - time for waiting if data doesn't appear
     * @param queue - queue instance for fetching
     * @param ref_for_insertion - reference to insert object
     * @param wait - time of waiting
     * @param error_reason - reason if there is no appeared object at all
     */
    template <typename Queue, typename ObjectType, typename WaitTime>
    void fetchFromQueue(Queue &queue,
                        ObjectType &ref_for_insertion,
                        const WaitTime &wait,
                        const std::string &error_reason);

    tbb::concurrent_queue<ProposalType> proposal_queue_;
    tbb::concurrent_queue<ProposalType> verified_proposal_queue_;
    tbb::concurrent_queue<BlockType> block_queue_;

    std::map<std::string, tbb::concurrent_queue<TxResponseType>>
        responses_queues_;
    std::shared_ptr<IrohaInstance> iroha_instance_;
    torii::CommandSyncClient command_client_;
    torii_utils::QuerySyncClient query_client_;

    void initPipeline(const shared_model::crypto::Keypair &keypair);
    void subscribeQueuesAndRun();

    // config area

    /// maximum time of waiting before appearing next proposal
    // TODO 21/12/2017 muratovv make relation of time with instance's config
    milliseconds proposal_waiting;

    /// maximum time of waiting before appearing next committed block
    milliseconds block_waiting;

    /// maximum time of waiting before appearing next transaction response
    milliseconds tx_response_waiting;

    size_t maximum_proposal_size_;

   private:
    logger::Logger log_ = logger::log("IntegrationTestFramework");
    std::mutex queue_mu;
    std::condition_variable queue_cond;
    std::function<void(IntegrationTestFramework &)> deleter_;
  };

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
