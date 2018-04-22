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

#include "module/shared_model/validators/validators_fixture.hpp"

#include <gtest/gtest.h>
#include <boost/range/irange.hpp>

#include <type_traits>
#include "builders/protobuf/transaction.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "utils/polymorphic_wrapper.hpp"

using namespace shared_model;

class TransactionValidatorTest : public ValidatorsTest {
 protected:
  iroha::protocol::Transaction generateEmptyTransaction() {
    std::string creator_account_id = "admin@test";

    TestTransactionBuilder builder;
    auto tx = builder.creatorAccountId(creator_account_id)
                  .createdTime(created_time)
                  .build()
                  .getTransport();
    return tx;
  }
};

/**
 * @given transaction without any commands
 * @when commands validator is invoked
 * @then answer has error about empty transaction
 */
TEST_F(TransactionValidatorTest, EmptyTransactionTest) {
  auto tx = generateEmptyTransaction();
  tx.mutable_payload()->set_created_time(created_time);
  shared_model::validation::DefaultTransactionValidator transaction_validator;
  auto result = proto::Transaction(iroha::protocol::Transaction(tx));
  auto answer =
      transaction_validator.validate(result);
  ASSERT_EQ(answer.getReasonsMap().size(), 1);
}

/**
 * @given transaction made of commands with valid fields
 * @when commands validation is invoked
 * @then answer has no errors
 */
TEST_F(TransactionValidatorTest, StatelessValidTest) {
  iroha::protocol::Transaction tx = generateEmptyTransaction();
  tx.mutable_payload()->set_creator_account_id(account_id);
  tx.mutable_payload()->set_created_time(created_time);
  auto payload = tx.mutable_payload();

  // Iterate through all command types, filling command fields with valid values
  iterateContainer([] { return iroha::protocol::Command::descriptor(); },
                   [&](auto field) {
                     // Add new command to transaction
                     auto command = payload->add_commands();
                     // Set concrete type for new command
                     return command->GetReflection()->MutableMessage(command,
                                                                     field);
                   },
                   [this](auto field, auto command) {
                     // Will throw key exception in case new field is added
                     field_setters.at(field->name())(
                         command->GetReflection(), command, field);
                   },
                   [] {});

  shared_model::validation::DefaultTransactionValidator transaction_validator;
  auto result = proto::Transaction(iroha::protocol::Transaction(tx));
  auto answer =
      transaction_validator.validate(result);

  ASSERT_FALSE(answer.hasErrors()) << answer.reason();
}

/**
 * @given transaction made of commands with invalid fields
 * @when commands validation is invoked
 * @then answer has errors and number of errors in answer is the same as the
 * number of commands in tx
 */
TEST_F(TransactionValidatorTest, StatelessInvalidTest) {
  iroha::protocol::Transaction tx = generateEmptyTransaction();
  auto payload = tx.mutable_payload();

  iroha::ts64_t invalid_time = 10000000000ull;
  payload->set_created_time(invalid_time);

  // create commands from default constructors, which will have empty, therefore
  // invalid values
  iterateContainer([] { return iroha::protocol::Command::descriptor(); },
                   [&](auto field) {
                     // Add new command to transaction
                     auto command = payload->add_commands();
                     // Set concrete type for new command
                     return command->GetReflection()->MutableMessage(command,
                                                                     field);
                   },
                   [](auto, auto) {
                     // Note that no fields are set
                   },
                   [] {});

  shared_model::validation::DefaultTransactionValidator transaction_validator;
  auto result = proto::Transaction(iroha::protocol::Transaction(tx));
  auto answer =
      transaction_validator.validate(result);

  // in total there should be number_of_commands + 1 reasons of bad answer:
  // number_of_commands for each command + 1 for transaction metadata
  ASSERT_EQ(answer.getReasonsMap().size(),
            iroha::protocol::Command::descriptor()->field_count() + 1);
}
