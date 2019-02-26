/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/iroha_instance.hpp"

#include <cstdlib>
#include <sstream>

#include "ametsuchi/storage.hpp"
#include "cryptography/keypair.hpp"
#include "framework/config_helper.hpp"
#include "framework/integration_framework/test_irohad.hpp"
#include "logger/logger.hpp"

using namespace std::chrono_literals;

namespace integration_framework {

  IrohaInstance::IrohaInstance(bool mst_support,
                               const std::string &block_store_path,
                               const std::string &listen_ip,
                               size_t torii_port,
                               size_t internal_port,
                               logger::LoggerManagerTreePtr irohad_log_manager,
                               logger::LoggerPtr log,
                               const boost::optional<std::string> &dbname)
      : block_store_dir_(block_store_path),
        pg_conn_(getPostgreCredsOrDefault(dbname)),
        listen_ip_(listen_ip),
        torii_port_(torii_port),
        internal_port_(internal_port),
        // proposal_timeout results in non-deterministic behavior due
        // to thread scheduling and network
        proposal_delay_(1h),
        // not required due to solo consensus
        vote_delay_(0ms),
        // amount of minutes in a day
        mst_expiration_time_(std::chrono::minutes(24 * 60)),
        opt_mst_gossip_params_(boost::make_optional(
            mst_support, iroha::GossipPropagationStrategyParams{})),
        irohad_log_manager_(std::move(irohad_log_manager)),
        log_(std::move(log)) {}

  void IrohaInstance::makeGenesis(const shared_model::interface::Block &block) {
    instance_->storage->reset();
    rawInsertBlock(block);
    instance_->init();
  }

  void IrohaInstance::rawInsertBlock(
      const shared_model::interface::Block &block) {
    instance_->storage->insertBlock(block);
  }

  void IrohaInstance::setMstGossipParams(
      std::chrono::milliseconds mst_gossip_emitting_period,
      uint32_t mst_gossip_amount_per_once) {
    BOOST_ASSERT_MSG(
        not instance_,
        "Gossip propagation params must be set before Irohad is started!");
    iroha::GossipPropagationStrategyParams gossip_params;
    gossip_params.emission_period = mst_gossip_emitting_period;
    gossip_params.amount_per_once = mst_gossip_amount_per_once;
    opt_mst_gossip_params_ = gossip_params;
  }

  void IrohaInstance::initPipeline(
      const shared_model::crypto::Keypair &key_pair, size_t max_proposal_size) {
    instance_ = std::make_shared<TestIrohad>(block_store_dir_,
                                             pg_conn_,
                                             listen_ip_,
                                             torii_port_,
                                             internal_port_,
                                             max_proposal_size,
                                             proposal_delay_,
                                             vote_delay_,
                                             mst_expiration_time_,
                                             key_pair,
                                             irohad_log_manager_,
                                             log_,
                                             opt_mst_gossip_params_);
  }

  void IrohaInstance::run() {
    instance_->run().match(
        [](const Irohad::RunResult::ValueType &) {},
        [](const Irohad::RunResult::ErrorType &error) {
          BOOST_THROW_EXCEPTION(std::runtime_error(error.error));
        });
  }

  std::shared_ptr<TestIrohad> &IrohaInstance::getIrohaInstance() {
    return instance_;
  }

  std::string IrohaInstance::getPostgreCredsOrDefault(
      const boost::optional<std::string> &dbname) {
    std::string db = " dbname=";
    if (dbname) {
      db += dbname.value();
    } else {
      db += "db"
          + boost::uuids::to_string(boost::uuids::random_generator()())
                .substr(0, 8);
    }

    return integration_framework::getPostgresCredsOrDefault() + db;
  }

}  // namespace integration_framework
