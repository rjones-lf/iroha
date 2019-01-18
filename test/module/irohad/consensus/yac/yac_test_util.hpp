/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_TEST_UTIL_HPP
#define IROHA_YAC_TEST_UTIL_HPP

#include <gmock/gmock.h>

#include "consensus/yac/vote_message.hpp"
#include "consensus/yac/yac_hash_provider.hpp"

#include "module/irohad/consensus/yac/mock_yac_crypto_provider.hpp"
#include "module/shared_model/interface_mocks.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      inline std::shared_ptr<shared_model::interface::Peer> mk_peer(
          const std::string &address) {
        auto key = std::string(32, '0');
        std::copy(address.begin(), address.end(), key.begin());
        auto peer = std::make_shared<MockPeer>();
        EXPECT_CALL(*peer, address())
            .WillRepeatedly(::testing::ReturnRefOfCopy(address));
        EXPECT_CALL(*peer, pubkey())
            .WillRepeatedly(::testing::ReturnRefOfCopy(
                shared_model::interface::types::PubkeyType(key)));

        return peer;
      }

      inline VoteMessage createVote(YacHash hash, const std::string &pub_key) {
        VoteMessage vote;
        vote.hash = std::move(hash);
        vote.signature = createSig(pub_key);
        return vote;
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_TEST_UTIL_HPP
