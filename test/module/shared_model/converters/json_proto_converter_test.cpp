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

#include <gtest/gtest.h>

#include "backend/protobuf/transaction.hpp"
#include "converters/protobuf/json_proto_converter.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model::proto;
using namespace shared_model::converters::protobuf;
using namespace shared_model;

/**
 * @given sample transaction shared model object
 * @when transaction is converted to json and then converted back to shared
 * model object
 * @then original and obtained objects are equal
 */
TEST(JsonProtoConverterTest, JsonToProtoTxTest) {
  TestTransactionBuilder builder;

  shared_model::interface::types::CounterType tx_counter = 1;
  std::string creator_account_id = "admin@test";

  auto orig_tx = builder.txCounter(tx_counter)
                     .creatorAccountId(creator_account_id)
                     .createdTime(123)
                     .build();

  auto json = modelToJson(orig_tx);

  auto obtained_tx = jsonToModel<shared_model::proto::Transaction>(json);
  ASSERT_TRUE(obtained_tx);
  ASSERT_EQ(orig_tx.getTransport().SerializeAsString(),
            obtained_tx.value().getTransport().SerializeAsString());

  // check some field's values
  ASSERT_EQ(orig_tx.createdTime(), obtained_tx->createdTime());
  ASSERT_EQ(orig_tx.creatorAccountId(), obtained_tx->creatorAccountId());
}

/**
 * @given invalid json string
 * @when json is converted to shared model object
 * @then none is returned
 */
TEST(JsonProtoConverterTest, InvalidJsonToProtoTx) {
  std::string json = "not json string";

  auto obtained_tx = jsonToModel<shared_model::proto::Transaction>(json);
  ASSERT_FALSE(obtained_tx);
}

/**
 * @given sample block shared model object
 * @when block is converted to json and then converted back to shared model
 * object
 * @then original and obtained objects are equal
 */
TEST(JsonProtoConverterTest, JsonToProtoBlockTest) {
  TestTransactionBuilder tx_builder;
  TestBlockBuilder block_builder;

  auto orig_block =
      block_builder.transactions(std::vector<Transaction>({tx_builder.build()}))
          .createdTime(123)
          .build();

  auto json = modelToJson(orig_block);

  auto obtained_block = jsonToModel<shared_model::proto::Block>(json);
  ASSERT_TRUE(obtained_block);
  ASSERT_EQ(orig_block.getTransport().SerializeAsString(),
            obtained_block.value().getTransport().SerializeAsString());
}
