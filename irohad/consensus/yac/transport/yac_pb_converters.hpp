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

#include "builders/default_builders.hpp"
#include "common/byteutils.hpp"
#include "consensus/yac/messages.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "interfaces/common_objects/signature.hpp"
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

          // Will fix it in the next PR, very soon, don't worry
          if (vote.hash.block_signature == nullptr) {
            auto peer_key = shared_model::crypto::DefaultCryptoAlgorithmType::
                                generateKeypair()
                                    .publicKey();
            shared_model::builder::DefaultSignatureBuilder()
                .publicKey(peer_key)
                .signedData(shared_model::crypto::Signed(""))
                .build()
                .match(
                    [&](iroha::expected::Value<std::shared_ptr<
                            shared_model::interface::Signature>> &sig) {
                      const_cast<VoteMessage &>(vote).hash.block_signature =
                          sig.value;
                    },
                    [](iroha::expected::Error<std::shared_ptr<std::string>>) {
                    });
          }

          block_signature->set_signature(shared_model::crypto::toBinaryString(
              vote.hash.block_signature->signedData()));

          block_signature->set_pubkey(shared_model::crypto::toBinaryString(
              vote.hash.block_signature->publicKey()));

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

          shared_model::builder::DefaultSignatureBuilder()
              .publicKey(shared_model::crypto::PublicKey(
                  pb_vote.hash().block_signature().pubkey()))
              .signedData(shared_model::crypto::Signed(
                  pb_vote.hash().block_signature().signature()))
              .build()
              .match(
                  [&](iroha::expected::Value<
                      std::shared_ptr<shared_model::interface::Signature>>
                          &sig) { vote.hash.block_signature = sig.value; },
                  [](iroha::expected::Error<std::shared_ptr<std::string>>) {});

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
