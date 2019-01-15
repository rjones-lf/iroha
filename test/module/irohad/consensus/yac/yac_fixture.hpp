/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_MOCKS_HPP
#define IROHA_YAC_MOCKS_HPP

#include <gtest/gtest.h>

#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/yac.hpp"

#include "module/irohad/consensus/yac/mocks/crypto_provider.hpp"
#include "module/irohad/consensus/yac/mocks/network.hpp"
#include "module/irohad/consensus/yac/mocks/timer.hpp"
#include "module/irohad/consensus/yac/yac_test_util.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacTest : public ::testing::Test {
       public:
        // ------|Network|------
        std::shared_ptr<MockYacNetwork> network;
        std::shared_ptr<MockYacCryptoProvider> crypto;
        std::shared_ptr<MockTimer> timer;
        std::shared_ptr<Yac> yac;

        // ------|Round|------
        std::vector<std::shared_ptr<shared_model::interface::Peer>>
            default_peers = [] {
              std::vector<std::shared_ptr<shared_model::interface::Peer>>
                  result;
              for (size_t i = 1; i <= 7; ++i) {
                result.push_back(mk_peer(std::to_string(i)));
              }
              return result;
            }();

        void SetUp() override {
          network = std::make_shared<MockYacNetwork>();
          crypto = std::make_shared<MockYacCryptoProvider>();
          timer = std::make_shared<MockTimer>();
          auto ordering = ClusterOrdering::create(default_peers);
          ASSERT_TRUE(ordering);
          initYac(ordering.value());
        }

        void TearDown() override {
          network->release();
        }

        void initYac(ClusterOrdering ordering) {
          yac = Yac::create(YacVoteStorage(), network, crypto, timer, ordering);
          network->subscribe(yac);
        }
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_MOCKS_HPP
