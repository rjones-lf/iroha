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

#include <rxcpp/rx-observable.hpp>

namespace shared_model {
  namespace interface {
    class Block;
  }
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {
    class MutableStorage;
  }

  namespace validation {

    /**
     * ChainValidator is interface of chain validation,
     * that is required on commit step of consensus
     */
    class ChainValidator {
     public:
      virtual ~ChainValidator() = default;

      /**
       * Validate method provide chain validation for application it to ledger.
       *
       * Chain validation will validate all signatures of new blocks
       * and related meta information such as previous hash, height and
       * other meta information
       * @param commit - observable with all blocks, that should be applied
       * atomically
       * @param storage - storage that may be modified during loading
       * @return true if commit is valid, false otherwise
       */
      virtual bool validateChain(
          rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
              commit,
          ametsuchi::MutableStorage &storage) = 0;

      /**
       * Block validation will check if all signatures and meta-data are valid.
       * @param block - storage that may be modified during loading
       * @param storage -  storage that may be modified during block appliance
       * @return true if block is valid and can be applied, false otherwise
       */
      virtual bool validateBlock(const shared_model::interface::Block &block,
                                 ametsuchi::MutableStorage &storage) = 0;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_CHAIN_VALIDATOR_HPP
