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
#ifndef IROHA_CHAIN_VALIDATOR_IMPL_HPP
#define IROHA_CHAIN_VALIDATOR_IMPL_HPP

#include <memory>

#include "logger/logger.hpp"
#include "validation/chain_validator.hpp"

namespace iroha {

  namespace consensus {
    namespace yac {
      class SupermajorityChecker;
    }  // namespace yac
  }    // namespace consensus

  namespace validation {
    class ChainValidatorImpl : public ChainValidator {
     public:
      ChainValidatorImpl(std::shared_ptr<consensus::yac::SupermajorityChecker>
                             supermajority_checker);

      bool validateChain(
          rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
              blocks,
          ametsuchi::MutableStorage &storage) override;

      bool validateBlock(const shared_model::interface::Block &block,
                         ametsuchi::MutableStorage &storage) override;

     private:
      /**
       * Provide functions to check supermajority
       */
      std::shared_ptr<consensus::yac::SupermajorityChecker>
          supermajority_checker_;

      logger::Logger log_;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_CHAIN_VALIDATOR_IMPL_HPP
