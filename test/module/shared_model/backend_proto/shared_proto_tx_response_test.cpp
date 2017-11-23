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

// Quite hacky way to extract the class name
// probably not stable and should be improved
class ClassNameVisitor : public boost::static_visitor<const std::string> {
 public:
  template <typename T>
  const std::string operator()(const T &t) const {
    auto s = t->toString();
    // skip everything except class name
    return std::string(s.begin(), s.begin() + s.find(':'));
  }
};

class ProtoTxResponse : public testing::Test {
 public:
  void SetUp() override {}
  void SetUp(iroha::protocol::TxStatus status) {
    r.set_tx_status(status);
    proto = std::make_shared<shared_model::proto::TransactionResponse>(r);
  }

  iroha::protocol::ToriiResponse r;
  std::shared_ptr<shared_model::proto::TransactionResponse> proto;
  ClassNameVisitor visitor;
};

/**
 * The following test ensures that the tx response is deserialized properly
 */

TEST_F(ProtoTxResponse, StatelessFailedLoad) {
  SetUp(iroha::protocol::STATELESS_VALIDATION_FAILED);
  ASSERT_STREQ("StatelessFailedTxResponse",
               boost::apply_visitor(visitor, proto->get()).c_str());
}

TEST_F(ProtoTxResponse, StatelessValidLoad) {
  SetUp(iroha::protocol::STATELESS_VALIDATION_SUCCESS);
  ASSERT_STREQ("StatelessValidTxResponse",
               boost::apply_visitor(visitor, proto->get()).c_str());
}

TEST_F(ProtoTxResponse, StatefulFailedLoad) {
  SetUp(iroha::protocol::STATEFUL_VALIDATION_FAILED);
  ASSERT_STREQ("StatefulFailedTxResponse",
               boost::apply_visitor(visitor, proto->get()).c_str());
}

TEST_F(ProtoTxResponse, StatefulValidLoad) {
  SetUp(iroha::protocol::STATEFUL_VALIDATION_SUCCESS);
  ASSERT_STREQ("StatefulValidTxResponse",
               boost::apply_visitor(visitor, proto->get()).c_str());
}

TEST_F(ProtoTxResponse, CommittedLoad) {
  SetUp(iroha::protocol::COMMITTED);
  ASSERT_STREQ("CommittedTxResponse",
               boost::apply_visitor(visitor, proto->get()).c_str());
}

TEST_F(ProtoTxResponse, UnknownLoad) {
  SetUp(iroha::protocol::NOT_RECEIVED);
  ASSERT_STREQ("UnknownTxResponse",
               boost::apply_visitor(visitor, proto->get()).c_str());
}

TEST_F(ProtoTxResponse, RandomLoad) {
  SetUp(static_cast<iroha::protocol::TxStatus>(
      123));  // some random big enough value
  ASSERT_STREQ("UnknownTxResponse",
               boost::apply_visitor(visitor, proto->get()).c_str());
}
