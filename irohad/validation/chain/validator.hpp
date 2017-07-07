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

#ifndef IROHA_CHAIN_VALIDATOR_HPP
#define IROHA_CHAIN_VALIDATOR_HPP

#include <ametsuchi/mutable_storage.hpp>
#include <model/model.hpp>
#include <rxcpp/rx-observable.hpp>

namespace iroha {
  namespace validation {

    /**
     * ChainValidator is interface of chain validation,
     * that require on commit step of consensus
     */
    class ChainValidator {
     public:
      /**
       * Validate method provide chain validation for application it to ledger.
       *
       * Chain validation assumes that all signatures of new blocks will be
       * valid
       * and valid related meta information such as previous hash, height and
       * other meta information
       * @param blocks - observable with all blocks, that should be applied
       * simultaneously
       * @param storage - storage that may be modified during loading
       * @return true if commit is valid, false otherwise
       */
      virtual bool validate(rxcpp::observable<model::Block> &blocks,
                            ametsuchi::MutableStorage &storage) = 0;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_CHAIN_VALIDATOR_HPP
