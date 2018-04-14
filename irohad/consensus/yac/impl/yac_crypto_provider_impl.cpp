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

#include "consensus/yac/impl/yac_crypto_provider_impl.hpp"
#include "consensus/yac/transport/yac_pb_converters.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      CryptoProviderImpl::CryptoProviderImpl(const shared_model::crypto::Keypair &keypair)
          : keypair_(keypair) {}

      bool CryptoProviderImpl::verify(CommitMessage msg) {
        return std::all_of(
            std::begin(msg.votes),
            std::end(msg.votes),
            [this](const auto &vote) { return this->verify(vote); });
      }

      bool CryptoProviderImpl::verify(RejectMessage msg) {
        return std::all_of(
            std::begin(msg.votes),
            std::end(msg.votes),
            [this](const auto &vote) { return this->verify(vote); });
      }

      bool CryptoProviderImpl::verify(VoteMessage msg) {
        return iroha::verify(
            iroha::sha3_256(
                PbConverters::serializeVote(msg).hash().SerializeAsString())
                .to_string(),
            msg.signature.pubkey,
            msg.signature.signature);
      }

      VoteMessage CryptoProviderImpl::getVote(YacHash hash) {
        VoteMessage vote;
        vote.hash = hash;
        keypair_t keypair = *std::unique_ptr<keypair_t>(keypair_.makeOldModel());
        auto signature = iroha::sign(
            iroha::sha3_256(
                PbConverters::serializeVote(vote).hash().SerializeAsString())
                .to_string(),
            keypair.pubkey,
            keypair.privkey);
        vote.signature.signature = signature;
        vote.signature.pubkey = keypair.pubkey;
        return vote;
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
