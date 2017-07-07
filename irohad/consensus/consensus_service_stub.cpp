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

#include <consensus/consensus_service_stub.hpp>

namespace iroha {
  namespace consensus {

    using model::Transaction;
    using model::Proposal;
    using model::Block;

    rxcpp::observable<rxcpp::observable<model::Block>>
    ConsensusServiceStub::on_commit() {
      return commits_.get_observable();
    }

    void ConsensusServiceStub::vote_block(model::Block &block) {
      commits_.get_subscriber().on_next(rxcpp::observable<>::from(block));
    }

  }  // namespace consensus
}  // namespace iroha
