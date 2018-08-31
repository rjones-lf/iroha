/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_crypto_provider_impl.hpp"

#include <gtest/gtest.h>
#include "backend/protobuf/common_objects/proto_common_objects_factory.hpp"
#include "consensus/yac/messages.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "module/shared_model/builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "validators/field_validator.hpp"

const auto pubkey = std::string(32, '0');
const auto signed_data = std::string(32, '1');

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacCryptoProviderTest : public ::testing::Test {
       public:
        YacCryptoProviderTest()
            : keypair(shared_model::crypto::DefaultCryptoAlgorithmType::
                          generateKeypair()) {}

        void SetUp() override {
          crypto_provider =
              std::make_shared<CryptoProviderImpl>(keypair, factory);
        }

        const shared_model::crypto::Keypair keypair;
        std::shared_ptr<shared_model::proto::ProtoCommonObjectsFactory<
            shared_model::validation::FieldValidator>>
            factory =
                std::make_shared<shared_model::proto::ProtoCommonObjectsFactory<
                    shared_model::validation::FieldValidator>>();
        std::shared_ptr<CryptoProviderImpl> crypto_provider;
      };

      TEST_F(YacCryptoProviderTest, ValidWhenSameMessage) {
        YacHash hash("1", "1");
        auto sig = shared_model::proto::SignatureBuilder()
                       .publicKey(shared_model::crypto::PublicKey(pubkey))
                       .signedData(shared_model::crypto::Signed(signed_data))
                       .build();

        hash.block_signature = clone(sig);

        auto vote = crypto_provider->getVote(hash);

        ASSERT_TRUE(crypto_provider->verify(vote));
      }

      TEST_F(YacCryptoProviderTest, InvalidWhenMessageChanged) {
        YacHash hash("1", "1");
        auto sig = shared_model::proto::SignatureBuilder()
                       .publicKey(shared_model::crypto::PublicKey(pubkey))
                       .signedData(shared_model::crypto::Signed(signed_data))
                       .build();

        hash.block_signature = clone(sig);

        auto vote = crypto_provider->getVote(hash);

        vote.hash.block_hash = "hash changed";

        ASSERT_FALSE(crypto_provider->verify(vote));
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
