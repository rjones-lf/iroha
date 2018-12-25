/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_LOADER_FIXTURE_HPP
#define IROHA_BLOCK_LOADER_FIXTURE_HPP

#include <memory>

#include <gtest/gtest.h>
#include <libfuzzer/libfuzzer_macro.h>

#include "consensus/consensus_block_cache.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "network/impl/block_loader_service.hpp"

using namespace testing;

namespace fuzzing {

  struct BlockLoaderFixture {
    std::shared_ptr<NiceMock<iroha::ametsuchi::MockBlockQuery>> storage_;
    std::shared_ptr<NiceMock<iroha::ametsuchi::MockBlockQueryFactory>>
        block_query_factory_;
    std::shared_ptr<iroha::consensus::ConsensusResultCache> block_cache_;
    std::shared_ptr<iroha::network::BlockLoaderService> block_loader_service_;

    BlockLoaderFixture() {
      // BlockLoaderService produces lots of error logs about missing blocks
      spdlog::set_level(spdlog::level::critical);

      storage_ = std::make_shared<NiceMock<iroha::ametsuchi::MockBlockQuery>>();
      block_query_factory_ =
          std::make_shared<NiceMock<iroha::ametsuchi::MockBlockQueryFactory>>();
      block_cache_ = std::make_shared<iroha::consensus::ConsensusResultCache>();
      block_loader_service_ =
          std::make_shared<iroha::network::BlockLoaderService>(
              block_query_factory_, block_cache_);
      EXPECT_CALL(*block_query_factory_, createBlockQuery())
          .WillRepeatedly(Return(boost::make_optional(
              std::shared_ptr<iroha::ametsuchi::BlockQuery>(storage_))));
    }
  };

}  // namespace fuzzing

#endif  // IROHA_BLOCK_LOADER_FIXTURE_HPP
