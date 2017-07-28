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

#include "simulator/impl/simulator.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace iroha;
using ::testing::Return;
using ::testing::_;

class TemporaryFactoryMock : public ametsuchi::TemporaryFactory {
 public:
  std::unique_ptr<ametsuchi::TemporaryWsv> createTemporaryWsv() {
    return std::unique_ptr<ametsuchi::TemporaryWsv>(createTemporaryWsvProxy());
  }

  MOCK_METHOD0(createTemporaryWsvProxy, ametsuchi::TemporaryWsv*());
};

class StatefulValidatorMock : public validation::StatefulValidator {
 public:
  MOCK_METHOD2(validate, model::Proposal(const model::Proposal&,
                                         ametsuchi::TemporaryWsv&));
};

/**
 * Mock class for Ametsuchi queries on blocks, transactions
 */
class BlockQueryMock : public ametsuchi::BlockQuery {
 public:
  MOCK_METHOD1(getAccountTransactions,
               rxcpp::observable<iroha::model::Transaction>(std::string));

  MOCK_METHOD2(getBlocks,
               rxcpp::observable<iroha::model::Block>(uint32_t, uint32_t));
};

TEST(SimulatorTest, process_proposal) {
  StatefulValidatorMock validator;
  TemporaryFactoryMock factory;
  BlockQueryMock query;
  model::HashProviderImpl provider;

  auto simulator = simulator::Simulator(validator, factory, query, provider);

  auto txs = std::vector<model::Transaction>(2);
  auto proposal = model::Proposal(txs);
  proposal.height = 2;

  EXPECT_CALL(query, getBlocks(proposal.height - 1, proposal.height))
      .WillRepeatedly(
          Return(rxcpp::observable<>::create<model::Block>([&proposal](auto s) {
            auto block = model::Block();
            block.height = proposal.height - 1;
            s.on_next(block);
            s.on_completed();
          })));

  EXPECT_CALL(validator, validate(_, _))
      .WillRepeatedly(Return(proposal));

  simulator.process_proposal(proposal);

  simulator.on_verified_proposal().subscribe(
      [&proposal](auto verified_proposal) {
        ASSERT_TRUE(verified_proposal.has_value());
        ASSERT_EQ(verified_proposal.value().height, proposal.height);

      });
}
