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

#ifndef IROHA_IROHA_INSTANCE_HPP
#define IROHA_IROHA_INSTANCE_HPP

#include <cstdlib>

#include "integration/pipeline/test_irohad.hpp"
#include "main/raw_block_insertion.hpp"

namespace integration_framework {

  using namespace std::chrono_literals;

  /**
   * Class provides interfaces for running Iroha instance.
   */
  class IrohaInstance {
   public:
    /**
     * Save the block as genesis block.
     * @param block - block to be saved as genesis block
     */
    void insertGenesis(const iroha::model::Block &block) {
      instance_->storage->dropStorage();
      rawInsertBlock(block);
      instance_->init();
    }

    /**
     * Insert the block without validation.
     * @param block - block to be inserted
     */
    void rawInsertBlock(const iroha::model::Block &block) {
      iroha::main::BlockInserter inserter(instance_->storage);
      inserter.applyToLedger({block});
    }

    /**
     * Create instance.
     * @param key_pair
     */
    void initPipeline() {
      instance_ = std::make_shared<TestIrohad>(block_store_dir_,
                                               redis_host_,
                                               redis_port_,
                                               pg_conn_,
                                               torii_port_,
                                               internal_port_,
                                               max_proposal_size_,
                                               proposal_delay_,
                                               vote_delay_,
                                               load_delay_,
                                               keypair_);
    }

    /**
     * Run instance.
     */
    void run() {
      instance_->run();
    }

    /**
     * Get instance of Iroha.
     * @return Iroha instance.
     */
    auto &getIrohaInstance() {
      return instance_;
    }

    /**
     * Set block storage directory for Iroha server.
     * @param block_store_dir - director to store blocks
     */
    void setBlockStoreDir(const std::string &block_store_dir) {
      block_store_dir_ = block_store_dir;
    }

    /**
     * Set Redis host.
     * @param redis_host - redis host address
     */
    void setRedisHost(const std::string &redis_host) {
      redis_host_ = redis_host;
    }

    /**
     * Set PostgreSQL connection string as
     * @code
     * "host=host_address port=5432 user=username password=user_password"
     * @endcode
     * @param pg_conn - PostgreSQL connection string
     */
    void setPgConn(const std::string &pg_conn) {
      pg_conn_ = pg_conn;
    }

    /**
     * Set Torii port.
     * @param torii_port
     */
    void setToriiPort(const size_t torii_port) {
      torii_port_ = torii_port;
    }

    /**
     * Set internal port.
     * @param internal_port
     */
    void setInternalPort(const size_t internal_port) {
      internal_port_ = internal_port;
    }

    /**
     * Set maximum proposal size.
     * @param max_proposal_size
     */
    void setMaxProposalSize(const size_t max_proposal_size) {
      max_proposal_size_ = max_proposal_size;
    }

    /**
     * Set proposal delay.
     * @param proposal_delay
     */
    void setProposalDelay(const std::chrono::milliseconds &proposal_delay) {
      proposal_delay_ = proposal_delay;
    }

    /**
     * Set vote delay.
     * @param vote_delay
     */
    void setVoteDelay(const std::chrono::milliseconds &vote_delay) {
      vote_delay_ = vote_delay;
    }

    /**
     * Set load delay.
     * @param load_delay
     */
    void setLoadDelay(const std::chrono::milliseconds &load_delay) {
      load_delay_ = load_delay;
    }

    /**
     * Set keypair.
     * @param keypair
     */
    void setKeypair(const iroha::keypair_t &keypair) {
      keypair_ = keypair;
    }

    std::shared_ptr<TestIrohad> instance_;

    /**
     * Get Redis host.
     * @param default_host - default value
     * @return host defined in environment variable if exists, otherwise default
     */
    std::string getRedisHostOrDefault(
        const std::string &default_host = "localhost") {
      auto redis_host = std::getenv("IROHA_REDIS_HOST");
      return redis_host ? redis_host : default_host;
    }

    /**
     * Get Redis port.
     * @param default_port - default value
     * @return port defined in environment variable if exists, otherwise or default
     */
    size_t getRedisPortOrDefault(size_t default_port = 6379) {
      auto redis_port = std::getenv("IROHA_REDIS_PORT");
      return redis_port ? std::stoull(redis_port) : default_port;
    }

    /**
     * Get PostgreSQL credentials.
     * @return PostgreSQL credential string constructed from system
     * environment if exists, otherwise default value.
     */
    std::string getPostgreCredsOrDefault(const std::string &default_conn =
                                             "host=localhost port=5432 "
                                             "user=postgres "
                                             "password=mysecretpassword") {
      auto pg_host = std::getenv("IROHA_POSTGRES_HOST");
      auto pg_port = std::getenv("IROHA_POSTGRES_PORT");
      auto pg_user = std::getenv("IROHA_POSTGRES_USER");
      auto pg_pass = std::getenv("IROHA_POSTGRES_PASSWORD");
      if (not pg_host) {
        return default_conn;
      } else {
        std::stringstream ss;
        ss << "host=" << pg_host << " port=" << pg_port << " user=" << pg_user
           << " password=" << pg_pass;
        return ss.str();
      }
    }

   private:
    // config area
    std::string block_store_dir_ = "/tmp/block_store";
    std::string redis_host_ = getRedisHostOrDefault();
    size_t redis_port_ = getRedisPortOrDefault();
    std::string pg_conn_ = getPostgreCredsOrDefault();
    size_t torii_port_ = 11501;
    size_t internal_port_ = 10001;
    size_t max_proposal_size_ = 10;
    std::chrono::milliseconds proposal_delay_ = 5000ms;
    std::chrono::milliseconds vote_delay_ = 5000ms;
    std::chrono::milliseconds load_delay_ = 5000ms;
    iroha::keypair_t keypair_;
  };
}  // namespace integration_framework
#endif  // IROHA_IROHA_INSTANCE_HPP
