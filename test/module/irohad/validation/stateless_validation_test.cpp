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

#include <gtest/gtest.h>
#include <chrono>
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "datetime/time.hpp"
#include "module/irohad/model/model_mocks.hpp"
#include "validation/impl/stateless_validator_impl.hpp"

using namespace std::chrono_literals;
using ::testing::A;
using ::testing::Return;

iroha::model::Transaction create_transaction() {
  iroha::model::Transaction tx{};
  tx.creator_account_id = "test";

  tx.tx_counter = 0;

  auto ts = iroha::time::now();
  tx.created_ts = ts;
  return tx;
}

TEST(stateless_validation, stateless_validation_when_valid) {
  spdlog::set_level(spdlog::level::off);

  auto crypto_provider = std::make_shared<iroha::model::MockCryptoProvider>();
  iroha::validation::StatelessValidatorImpl transaction_validator(
      crypto_provider);

  auto tx = create_transaction();

  EXPECT_CALL(*crypto_provider, verify(A<const iroha::model::Transaction &>()))
      .WillRepeatedly(Return(true));

  ASSERT_TRUE(transaction_validator.validate(tx));
}

TEST(stateless_validation, stateless_validation_when_invalid_wrong_signature) {
  spdlog::set_level(spdlog::level::off);

  auto crypto_provider = std::make_shared<iroha::model::MockCryptoProvider>();
  iroha::validation::StatelessValidatorImpl transaction_validator(
      crypto_provider);

  auto tx = create_transaction();

  EXPECT_CALL(*crypto_provider, verify(A<const iroha::model::Transaction &>()))
      .WillRepeatedly(Return(false));

  ASSERT_FALSE(transaction_validator.validate(tx));
}

TEST(stateless_validation,
     stateless_validation_when_invalid_due_to_big_time_delay) {
  spdlog::set_level(spdlog::level::off);

  auto crypto_provider = std::make_shared<iroha::model::MockCryptoProvider>();
  iroha::validation::StatelessValidatorImpl transaction_validator(
      crypto_provider);

  auto tx = create_transaction();

  EXPECT_CALL(*crypto_provider, verify(A<const iroha::model::Transaction &>()))
      .WillRepeatedly(Return(true));

  auto ts = iroha::time::now(-25h);
  tx.created_ts = ts;  // tx created 25 hours ago

  ASSERT_FALSE(transaction_validator.validate(tx));
}

TEST(stateless_validation,
     stateless_validation_when_invalid_due_to_tx_from_future) {
  spdlog::set_level(spdlog::level::off);

  auto crypto_provider = std::make_shared<iroha::model::MockCryptoProvider>();
  iroha::validation::StatelessValidatorImpl transaction_validator(
      crypto_provider);

  auto tx = create_transaction();

  EXPECT_CALL(*crypto_provider, verify(A<const iroha::model::Transaction &>()))
      .WillRepeatedly(Return(true));

  auto ts = iroha::time::now(1h);
  tx.created_ts = ts;  // tx created 1 hour later

  ASSERT_FALSE(transaction_validator.validate(tx));
}
