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

#ifndef IROHA_IROHA_INSTANCE_HPP
#define IROHA_IROHA_INSTANCE_HPP

#include <chrono>
#include <string>

#include "integration/pipeline/test_irohad.hpp"
#include "cryptography/keypair.hpp"

namespace integration_framework {

  class IrohaInstance {
   public:
    IrohaInstance();

    void makeGenesis(const shared_model::interface::Block &block);

    void rawInsertBlock(const shared_model::interface::Block &block);
    void initPipeline(const shared_model::crypto::Keypair &key_pair,
                      size_t max_proposal_size = 10);

    void run();

    std::shared_ptr<TestIrohad> &getIrohaInstance();

    std::string getPostgreCredsOrDefault(const std::string &default_conn =
                                             "host=localhost port=5432 "
                                             "user=postgres "
                                             "password=mysecretpassword");

    std::shared_ptr<TestIrohad> instance_;

    // config area
    const std::string block_store_dir_;
    const std::string pg_conn_;
    const size_t torii_port_;
    const size_t internal_port_;
    const std::chrono::milliseconds proposal_delay_;
    const std::chrono::milliseconds vote_delay_;
    const std::chrono::milliseconds load_delay_;
  };
}  // namespace integration_framework
#endif  // IROHA_IROHA_INSTANCE_HPP
