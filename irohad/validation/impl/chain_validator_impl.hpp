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

#include "logger/logger.hpp"
#include "model/model_crypto_provider.hpp"
#include "validation/chain_validator.hpp"

namespace iroha {
  namespace validation {
    class ChainValidatorImpl : public ChainValidator {
     public:
      ChainValidatorImpl();

      bool validateChain(OldCommit blocks,
                         ametsuchi::MutableStorage &storage) override;

      bool validateBlock(const model::Block &block,
                         ametsuchi::MutableStorage &storage) override;

     private:
      logger::Logger log_;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_CHAIN_VALIDATOR_IMPL_HPP
