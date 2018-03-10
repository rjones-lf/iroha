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

#ifndef IROHA_PROPOSAL_HPP
#define IROHA_PROPOSAL_HPP

#include <model/transaction.hpp>
#include <vector>

namespace iroha {
  namespace model {

    /**
     * Proposal is a Model-structure that provide bunch of transactions emitted
     * by
     * ordering service.
     * Proposal has no signatures and other meta information.
     */
    struct Proposal {
      explicit Proposal(std::vector<Transaction> txs)
          : transactions(txs), height(0) {}

      /**
       * Bunch of transactions provided by ordering service.
       */
      const std::vector<Transaction> transactions{};

      /**
       * Height of current proposal.
       * Note: This height must be consistent with your last block height
       */
      uint64_t height{};

      /**
       * Time when the proposal have been created
       */
      uint64_t created_time{};

      bool operator==(const Proposal &rhs) const;
      bool operator!=(const Proposal &rhs) const;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_PROPOSAL_HPP
