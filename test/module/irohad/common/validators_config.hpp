/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef IROHA_VALIDATORS_CONFIG_HPP
#define IROHA_VALIDATORS_CONFIG_HPP

#include "validators/validators_common.hpp"

namespace iroha {
  namespace test {

    static const uint64_t kTestsMaxBatchSize(10000u);
    static const std::shared_ptr<shared_model::validation::ValidatorsConfig>
        kTestsValidatorsConfig(
            std::make_shared<shared_model::validation::ValidatorsConfig>(
                kTestsMaxBatchSize));

  }  // namespace test
}  // namespace iroha

#endif  // IROHA_VALIDATORS_CONFIG_HPP
