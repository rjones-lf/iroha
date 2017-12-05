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
  auto log = logger::testLog("HandlerTest");

  CommandRegistry registry;
  ASSERT_EQ(15, registry.command_handler.types().size());
}

TEST(HandlerTest, QueryRegistration) {
  auto log = logger::testLog("HandlerTest");

  QueryRegistry registry;

  ASSERT_EQ(9, registry.query_handler.types().size());
}

TEST(HandlerTest, QueryResponseRegistration) {
  auto log = logger::testLog("HandlerTest");

  QueryResponseRegistry registry;

  ASSERT_EQ(8, registry.query_response_handler.types().size());
}

TEST(HandlerTest, TransactionResponseRegistration) {
  auto log = logger::testLog("HandlerTest");

  TransactionResponseRegistry registry;

  ASSERT_EQ(1, registry.transaction_response_handler.types().size());
}

