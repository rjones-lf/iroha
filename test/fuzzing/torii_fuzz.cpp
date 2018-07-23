/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <memory>
#include "libfuzzer/libfuzzer_macro.h"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/multi_sig_transactions/mst_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "torii/command_service.hpp"
#include "torii/processor/transaction_processor_impl.hpp"

using namespace std::chrono_literals;
using testing::Return;

struct CommandFixture {
  std::shared_ptr<torii::CommandService> service_;
  std::shared_ptr<iroha::torii::TransactionProcessorImpl> tx_processor_;
  std::shared_ptr<iroha::network::MockPeerCommunicationService> pcs_;
  std::shared_ptr<iroha::MockMstProcessor> mst_processor_;

  rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Proposal>>
      prop_notifier_;
  rxcpp::subjects::subject<
      std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
      vprop_notifier_;
  rxcpp::subjects::subject<iroha::Commit> commit_notifier_;
  rxcpp::subjects::subject<iroha::DataType> mst_notifier_;

  CommandFixture() {
    pcs_ = std::make_shared<iroha::network::MockPeerCommunicationService>();
    EXPECT_CALL(*pcs_, on_proposal())
        .WillRepeatedly(Return(prop_notifier_.get_observable()));
    EXPECT_CALL(*pcs_, on_commit())
        .WillRepeatedly(Return(commit_notifier_.get_observable()));
    EXPECT_CALL(*pcs_, on_verified_proposal())
        .WillRepeatedly(Return(vprop_notifier_.get_observable()));

    mst_processor_ = std::make_shared<iroha::MockMstProcessor>();
    EXPECT_CALL(*mst_processor_, onPreparedTransactionsImpl())
        .WillRepeatedly(Return(mst_notifier_.get_observable()));
    EXPECT_CALL(*mst_processor_, onExpiredTransactionsImpl())
        .WillRepeatedly(Return(mst_notifier_.get_observable()));

    tx_processor_ = std::make_shared<iroha::torii::TransactionProcessorImpl>(
        pcs_, mst_processor_);
    service_ = std::make_shared<torii::CommandService>(
        tx_processor_, nullptr, 15s, 15s);
  }
};

DEFINE_BINARY_PROTO_FUZZER(const iroha::protocol::Transaction &tx) {
  static CommandFixture handler;
  handler.service_->Torii(tx);
}
