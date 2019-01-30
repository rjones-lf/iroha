/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MOCK_YAC_CRYPTO_PROVIDER_HPP
#define IROHA_MOCK_YAC_CRYPTO_PROVIDER_HPP

#include <gmock/gmock.h>

#include "consensus/yac/yac_crypto_provider.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"

#include "module/shared_model/interface_mocks.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * Creates test signature with empty signed data, and provided pubkey
       * @param pub_key - public key to put in the signature
       * @return new signature
       */
      std::shared_ptr<shared_model::interface::Signature> createSig(
          const std::string &pub_key) {
        auto tmp =
            shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()
                .publicKey();
        std::string key(tmp.blob().size(), 0);
        std::copy(pub_key.begin(), pub_key.end(), key.begin());
        auto sig = std::make_shared<MockSignature>();
        EXPECT_CALL(*sig, publicKey())
            .WillRepeatedly(::testing::ReturnRefOfCopy(
                shared_model::crypto::PublicKey(key)));
        EXPECT_CALL(*sig, signedData())
            .WillRepeatedly(
                ::testing::ReturnRefOfCopy(shared_model::crypto::Signed("")));

        return sig;
      }

      class MockYacCryptoProvider : public YacCryptoProvider {
       public:
        MOCK_METHOD1(verify, bool(const std::vector<VoteMessage> &));

        VoteMessage getVote(YacHash hash) override {
          VoteMessage vote;
          vote.hash = std::move(hash);
          vote.signature = createSig("");
          return vote;
        }

        MockYacCryptoProvider() = default;

        MockYacCryptoProvider(const MockYacCryptoProvider &) {}

        MockYacCryptoProvider &operator=(const MockYacCryptoProvider &) {
          return *this;
        }
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_MOCK_YAC_CRYPTO_PROVIDER_HPP
