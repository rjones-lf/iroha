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

#include "framework/integration_framework/iroha_instance.hpp"
#include <cstdlib>
#include <sstream>
#include "cryptography/keypair.hpp"
#include "framework/integration_framework/test_irohad.hpp"

using namespace std::chrono_literals;

namespace integration_framework {

  IrohaInstance::IrohaInstance(bool mst_support, const std::string &block_store_path)
      : block_store_dir_(block_store_path),
        pg_conn_(getPostgreCredsOrDefault()),
        torii_port_(11501),
        internal_port_(50541),
        // proposal_timeout results in non-deterministic behavior due
        // to thread scheduling and network
        proposal_delay_(1h),
        // not required due to solo consensus
        vote_delay_(0ms),
        // same as above
        load_delay_(0ms),
        is_mst_supported_(mst_support) {}

  void IrohaInstance::makeGenesis(const shared_model::interface::Block &block) {
    instance_->storage->dropStorage();
    rawInsertBlock(block);
    instance_->init();
  }

  void IrohaInstance::rawInsertBlock(
      const shared_model::interface::Block &block) {
    instance_->storage->insertBlock(block);
  }

  void IrohaInstance::initPipeline(
      const shared_model::crypto::Keypair &key_pair, size_t max_proposal_size) {
    instance_ = std::make_shared<TestIrohad>(block_store_dir_,
                                             pg_conn_,
                                             torii_port_,
                                             internal_port_,
                                             max_proposal_size,
                                             proposal_delay_,
                                             vote_delay_,
                                             load_delay_,
                                             key_pair,
                                             is_mst_supported_);
  }

  void IrohaInstance::run() {
    instance_->run();
  }

  std::shared_ptr<TestIrohad> &IrohaInstance::getIrohaInstance() {
    return instance_;
  }

  std::string IrohaInstance::getPostgreCredsOrDefault(
      const std::string &default_conn) {
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

}  // namespace integration_framework
