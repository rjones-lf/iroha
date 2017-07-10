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

#ifndef IROHA_TRANSACTION_VALIDATOR_IMPL_HPP
#define IROHA_TRANSACTION_VALIDATOR_IMPL_HPP

#include <model/model.hpp>
#include <validation/stateless/validator.hpp>

namespace iroha {
  namespace validation {
    class StatelessValidatorImpl : public StatelessValidator {
     public:
      explicit StatelessValidatorImpl(
          model::ModelCryptoProvider& crypto_provider);
      bool validate(const model::Transaction& transaction) const override;

     private:
      static constexpr uint64_t MAX_DELAY =
          1000 * 3600 * 24;  // max-delay between tx creation and validation
      const model::ModelCryptoProvider& crypto_provider_;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_TRANSACTION_VALIDATOR_IMPL_HPP
