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

#ifndef IROHA_SYNCHRONIZER_HPP
#define IROHA_SYNCHRONIZER_HPP

#include <rxcpp/rx-observable.hpp>

#include "interfaces/iroha_internal/block_variant.hpp"
#include "network/peer_communication_service.hpp"

namespace iroha {
  namespace synchronizer {

    /**
     * Synchronizer is interface for fetching missed blocks
     */
    class Synchronizer {
     public:
      /**
       * Processing last committed block
       */
      virtual void process_commit(
          const shared_model::interface::BlockVariant &commit_message) = 0;

      /**
       * Emit committed blocks
       * Note: from one block received on consensus
       */
      virtual rxcpp::observable<Commit> on_commit_chain() = 0;

      virtual ~Synchronizer() = default;
    };
  }  // namespace synchronizer
}  // namespace iroha
#endif  // IROHA_SYNCHRONIZER_HPP
