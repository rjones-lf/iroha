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

#include "cryptography/ed25519_sha3_impl/crypto_provider.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "cryptography/ed25519_sha3_impl/signer.hpp"
#include "cryptography/ed25519_sha3_impl/verifier.hpp"

namespace shared_model {
  namespace crypto {

    Signed CryptoProviderEd25519Sha3::sign(const Blob &blob,
                                           const Keypair &keypair) {
      return Signer::sign(blob, keypair);
    }

    bool CryptoProviderEd25519Sha3::verify(const Signed &signedData,
                                           const Blob &orig,
                                           const PublicKey &publicKey) {
      return Verifier::verify(signedData, orig, publicKey);
    }

    Seed CryptoProviderEd25519Sha3::generateSeed() {
      return Seed(iroha::create_seed().to_string());
    }

    Seed CryptoProviderEd25519Sha3::generateSeed(
        const std::string &passphrase) {
      return Seed(iroha::create_seed(passphrase).to_string());
    }

    Keypair CryptoProviderEd25519Sha3::generateKeypair() {
      return generateKeypair(generateSeed());
    }

    Keypair CryptoProviderEd25519Sha3::generateKeypair(const Seed &seed) {
      auto keypair = iroha::create_keypair(
          iroha::blob_t<32>::from_string(toBinaryString(seed)));
      return Keypair(PublicKey(keypair.pubkey.to_string()),
                     PrivateKey(keypair.privkey.to_string()));
    }

    const size_t CryptoProviderEd25519Sha3::kHashLength = 256 / 8;
    const size_t CryptoProviderEd25519Sha3::kPublicKeyLength = 256 / 8;
    const size_t CryptoProviderEd25519Sha3::kPublicKeyHexLength = kPublicKeyLength * 2;
    const size_t CryptoProviderEd25519Sha3::kPrivateKeyLength = 512 / 8;
    const size_t CryptoProviderEd25519Sha3::kSignatureLength = 512 / 8;
    const size_t CryptoProviderEd25519Sha3::kSeedLength = 256 / 8;
  }  // namespace crypto
}  // namespace shared_model
