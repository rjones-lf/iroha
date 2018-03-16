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

#ifndef IROHA_YAC_PB_CONVERTERS_HPP
#define IROHA_YAC_PB_CONVERTERS_HPP

#include "common/byteutils.hpp"
#include "consensus/yac/messages.hpp"
#include "yac.pb.h"

namespace iroha {
  namespace consensus {
    namespace yac {
      class PbConverters {
       public:
        static proto::Vote serializeVote(const VoteMessage &vote) {
          proto::Vote pb_vote;

          auto hash = pb_vote.mutable_hash();
          hash->set_block(vote.hash.block_hash);
          hash->set_proposal(vote.hash.proposal_hash);

          auto block_signature = hash->mutable_block_signature();
          block_signature->set_signature(
              vote.hash.block_signature.signature.to_string());
          block_signature->set_pubkey(
              vote.hash.block_signature.pubkey.to_string());

          auto signature = pb_vote.mutable_signature();
          signature->set_signature(vote.signature.signature.to_string());
          signature->set_pubkey(vote.signature.pubkey.to_string());

          return pb_vote;
        }

        static boost::optional<VoteMessage> deserializeVote(
            const proto::Vote &pb_vote) {
          VoteMessage vote;
          vote.hash.proposal_hash = pb_vote.hash().proposal();
          vote.hash.block_hash = pb_vote.hash().block();
          vote.hash.block_signature.signature =
              *stringToBlob<iroha::sig_t::size()>(
                  pb_vote.hash().block_signature().signature());
          vote.hash.block_signature.pubkey =
              *stringToBlob<iroha::pubkey_t::size()>(
                  pb_vote.hash().block_signature().pubkey());
          vote.signature.signature = *stringToBlob<iroha::sig_t::size()>(
              pb_vote.signature().signature());
          vote.signature.pubkey = *stringToBlob<iroha::pubkey_t::size()>(
              pb_vote.signature().pubkey());

          return vote;
        }
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_PB_CONVERTERS_HPP
