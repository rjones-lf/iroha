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
#include "logger/logger.hpp"

#include "model/registration/command_registration.hpp"
#include "model/registration/query_registration.hpp"
#include "model/registration/query_response_registration.hpp"
#include "model/registration/transaction_response_registration.hpp"

using namespace std;
using namespace iroha::model;

TEST(HandlerTest, CommandRegistration) {
  auto log = logger::log("HandlerTest"); // todo switch with test logger

  CommandRegistry registry;

  log->info("Available command types: {}",
            logger::to_string(registry.command_handler.types(), [](auto index) {
              return index.name();
            }));

  ASSERT_EQ(11, registry.command_handler.types().size());
}

TEST(HandlerTest, QueryRegistration) {
  auto log = logger::log("HandlerTest"); // todo switch with test logger

  QueryRegistry registry;

  log->info("Available query types: {}",
            logger::to_string(registry.query_handler.types(), [](auto index) {
              return index.name();
            }));

  ASSERT_EQ(5, registry.query_handler.types().size());
}

TEST(HandlerTest, QueryResponseRegistration) {
  auto log = logger::log("HandlerTest"); // todo switch with test logger

  QueryResponseRegistry registry;

  log->info("Available query response types: {}",
            logger::to_string(registry.query_response_handler.types(), [](auto index) {
              return index.name();
            }));

  ASSERT_EQ(5, registry.query_response_handler.types().size());
}

TEST(HandlerTest, TransactionResponseRegistration) {
  auto log = logger::log("HandlerTest"); // todo switch with test logger

  TransactionResponseRegistry registry;

  log->info("Available transaction response types: {}",
            logger::to_string(registry.transaction_response_handler.types(), [](auto index) {
              return index.name();
            }));

  ASSERT_EQ(1, registry.transaction_response_handler.types().size());
}

