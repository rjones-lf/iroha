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
#include <boost/filesystem.hpp>
#include "backend/protobuf/from_old_model.hpp"

using namespace std::chrono_literals;

namespace integration_framework {

  IrohaInstance::IrohaInstance()
      : block_store_dir_(
            (boost::filesystem::temp_directory_path() / "block_store")
                .string()),
        pg_conn_(getPostgreCredsOrDefault()),
        torii_port_(11501),
        internal_port_(50541),
        proposal_delay_(5000ms),
        vote_delay_(5000ms),
        load_delay_(5000ms) {}

  void IrohaInstance::makeGenesis(const iroha::model::Block &block) {
    instance_->storage->dropStorage();
    rawInsertBlock(block);
    instance_->init();
  }

  void IrohaInstance::rawInsertBlock(const iroha::model::Block &block) {
    instance_->storage->insertBlock(shared_model::proto::from_old(block));
  }

  void IrohaInstance::initPipeline(const iroha::keypair_t &key_pair,
                                   size_t max_proposal_size) {
    keypair_ = key_pair;
    instance_ = std::make_shared<TestIrohad>(block_store_dir_,
                                             pg_conn_,
                                             torii_port_,
                                             internal_port_,
                                             max_proposal_size,
                                             proposal_delay_,
                                             vote_delay_,
                                             load_delay_,
                                             keypair_);
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
