/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_MODEL_CRYPTO_HPP
#define IROHA_SHARED_MODEL_MODEL_CRYPTO_HPP

#include "cryptography/blob.hpp"
#include "cryptography/keypair.hpp"
#include "cryptography/signed.hpp"

namespace shared_model {
  namespace bindings {
    /**
     * Class for crypto operations for SWIG
     */
    class ModelCrypto {
     public:
      /**
       * Generates new keypair (ed25519)
       * @return generated keypair
       */
      crypto::Keypair generateKeypair();

      /**
       * Creates keypair (ed25519) from provided private key
       * @param private_key - ed25519 hex-encoded private key with length 64
       * @return created keypair
       */
      crypto::Keypair fromPrivateKey(const std::string &private_key);

      /**
       * Retrieves Keypair object (ed25519) from existing keypair.
       * @param public_key - ed25519 hex-encoded public key
       * @param private_key - ed25519 hex-encoded private key
       * @return keypair from provided keys
       */
      crypto::Keypair convertFromExisting(const std::string &public_key,
                                          const std::string &private_key);
    };
  }  // namespace bindings
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_MODEL_CRYPTO_HPP
