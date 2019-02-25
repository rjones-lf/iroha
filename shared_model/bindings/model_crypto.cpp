/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bindings/model_crypto.hpp"
#include "common/byteutils.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "generator/generator.hpp"

namespace shared_model {
  namespace bindings {
    crypto::Keypair ModelCrypto::generateKeypair() {
      return crypto::CryptoProviderEd25519Sha3::generateKeypair();
    }

    crypto::Keypair ModelCrypto::fromPrivateKey(
        const std::string &private_key) {
      if (private_key.size()
          != crypto::DefaultCryptoAlgorithmType::kPrivateKeyLength) {
        throw std::invalid_argument("input string has incorrect length "
                                    + std::to_string(private_key.length()));
      }
      auto byte_string = iroha::hexstringToBytestring(private_key);
      if (not byte_string) {
        throw std::invalid_argument("invalid seed");
      }
      return crypto::CryptoProviderEd25519Sha3::generateKeypair(
          crypto::Seed(*byte_string));
    }

    crypto::Keypair ModelCrypto::convertFromExisting(
        const std::string &public_key, const std::string &private_key) {
      crypto::Keypair keypair((crypto::Keypair::PublicKeyType(
                                  crypto::Blob::fromHexString(public_key))),
                              crypto::Keypair::PrivateKeyType(
                                  crypto::Blob::fromHexString(private_key)));

      auto rand_str = generator::randomString(32);
      if (not crypto::CryptoProviderEd25519Sha3::verify(
              crypto::CryptoProviderEd25519Sha3::sign(crypto::Blob(rand_str),
                                                      keypair),
              crypto::Blob(rand_str),
              keypair.publicKey())) {
        throw std::invalid_argument("Provided keypair is not correct");
      }

      return keypair;
    }
  }  // namespace bindings
}  // namespace shared_model
