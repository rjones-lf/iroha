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

#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"

#include <gtest/gtest.h>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/irange.hpp>
#include "cryptography/hash.hpp"

/**
 * @given protobuf's ToriiResponse with different tx_statuses and some hash
 * @when converting to shared model
 * @then ensure that status and hash remain the same
 */
TEST(ProtoTxResponse, TxResponseLoad) {
  iroha::protocol::ToriiResponse response;
  const std::string hash = "123";
  response.set_tx_hash(hash);
  auto desc = response.GetDescriptor();
  auto tx_status = desc->FindFieldByName("tx_status");
  ASSERT_NE(nullptr, tx_status);
  auto tx_status_enum = tx_status->enum_type();
  ASSERT_NE(nullptr, tx_status_enum);

  boost::for_each(boost::irange(0, tx_status_enum->value_count()), [&](auto i) {
    response.GetReflection()->SetEnumValue(
        &response, tx_status, tx_status_enum->value(i)->number());
    auto model_response = shared_model::proto::TransactionResponse(response);
    ASSERT_EQ(i, model_response.get().which());
    ASSERT_EQ(model_response.transactionHash(),
              shared_model::crypto::Hash(hash));
  });
}
