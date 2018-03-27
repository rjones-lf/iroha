/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_VALIDATOR_MOCKS_HPP
#define IROHA_VALIDATOR_MOCKS_HPP

#include <gmock/gmock.h>
#include "interfaces/transaction.hpp"

namespace shared_model {
  namespace validation {

    // TODO: kamilsa 01.02.2018 IR-873 Replace all these validators with mock
    // classes

    template <typename Iface>
    struct AlwaysValidValidator {
      Answer validate(const Iface &) const {
        return {};
      }
    };
    using TransactionAlwaysValidValidator =
        AlwaysValidValidator<interface::Transaction>;
    using BlockAlwaysValidValidator = AlwaysValidValidator<interface::Block>;
    using ProposalAlwaysValidValidator =
        AlwaysValidValidator<interface::Proposal>;
    using QueryAlwaysValidValidator = AlwaysValidValidator<interface::Query>;

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_VALIDATOR_MOCKS_HPP
