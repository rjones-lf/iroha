/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "cryptography/crypto_provider/crypto_verifier.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      CryptoProviderImpl::CryptoProviderImpl(
          const shared_model::crypto::Keypair &keypair)
          : keypair_(keypair) {}

      bool CryptoProviderImpl::verify(std::vector<VoteMessage> msg) {
        return std::all_of(
            std::begin(msg), std::end(msg), [](const auto &vote) {
              auto serialized =
                  PbConverters::serializeVote(vote).hash().SerializeAsString();
              auto blob = shared_model::crypto::Blob(serialized);

              return shared_model::crypto::CryptoVerifier<>::verify(
                  vote.signature->signedData(),
                  blob,
                  vote.signature->publicKey());
            });
      }

      VoteMessage CryptoProviderImpl::getVote(YacHash hash) {
        VoteMessage vote;
        vote.hash = hash;
        auto serialized =
            PbConverters::serializeVotePayload(vote).hash().SerializeAsString();
        auto blob = shared_model::crypto::Blob(serialized);
        const auto &pubkey = keypair_.publicKey();
        const auto &privkey = keypair_.privateKey();
        auto signature = shared_model::crypto::CryptoSigner<>::sign(
            blob, shared_model::crypto::Keypair(pubkey, privkey));

        shared_model::builder::DefaultSignatureBuilder()
            .publicKey(pubkey)
            .signedData(signature)
            .build()
            .match([&vote](iroha::expected::Value<
                           std::shared_ptr<shared_model::interface::Signature>>
                               &sig) { vote.signature = sig.value; },
                   [](iroha::expected::Error<std::shared_ptr<std::string>>
                          &reason) {
                     logger::log("YacCryptoProvider::getVote")
                         ->error("Cannot build vote signature: {}",
                                 *reason.error);
                   });
        return vote;
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
