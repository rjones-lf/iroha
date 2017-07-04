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

#include <validation/stateless/validator_stub.hpp>
#include <validation/stateless/transaction_validator_stub.hpp>
#include <iostream>

namespace iroha {
  namespace validation {

    bool StatelessValidatorStub::validate(
        const dao::Transaction &transaction) const {
      std::cout << "[Stateless validation] validate transaction" << std::endl;
      TransactionValidatorStub validator;
      return validator.validate(transaction);
    }

    StatelessValidatorStub::StatelessValidatorStub(){}
  }  // namespace validation
}  // namespace iroha