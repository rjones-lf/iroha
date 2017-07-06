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

#ifndef IROHA_CONSENSUS_SERVICE_STUB_HPP
#define IROHA_CONSENSUS_SERVICE_STUB_HPP

#include <ametsuchi/ametsuchi.hpp>
#include <consensus/consensus_service.hpp>
#include <validation/chain/validator.hpp>
#include <validation/stateful/validator.hpp>

namespace iroha {
  namespace consensus {
    class ConsensusServiceStub : public ConsensusService {
     public:
      void vote_block(dao::Block &block) override;
      rxcpp::observable<rxcpp::observable<dao::Block>> on_commit() override;

     private:
      rxcpp::subjects::subject<rxcpp::observable<dao::Block>> commits_;
    };
  }  // namespace consensus
}  // namespace iroha

#endif  // IROHA_CONSENSUS_SERVICE_STUB_HPP
