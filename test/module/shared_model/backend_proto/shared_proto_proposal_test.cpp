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

#include "backend/protobuf/proposal.hpp"
#include "builders/protobuf/proposal.hpp"
#include "ordering.pb.h"
#include <gtest/gtest.h>

/**
 * @given protobuf transaction with transaction counter set
 * @when converted to shared model
 * @then shared model is created correctly
 */
TEST(ProtoTransaction, Create) {
  iroha::ordering::proto::Proposal proposal;

  shared_model::proto::Proposal proto(std::move(proposal));
  //ASSERT_EQ(proto.transactionCounter(), transaction.payload().tx_counter());
}