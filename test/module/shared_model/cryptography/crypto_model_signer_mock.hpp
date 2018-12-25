/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CRYPTO_MODEL_SIGNER_MOCK_HPP
#define IROHA_CRYPTO_MODEL_SIGNER_MOCK_HPP

#include <gmock/gmock.h>

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/queries/proto_query.hpp"
#include "backend/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"

namespace shared_model {
  namespace crypto {

    /**
     * Here we mock template method sign<>() from template class
     * CryptoModelSigner<>. We specialize interestd methods that call
     * crypto_signer_expecter which will catch interesting calls. Just use
     * crypto_signer_expecter in EXPECT_CALL() instead of CryptoModelSigner
     * instance. Since gtest consider global variable as memory leak,
     * we wrap crypto_signer_expecter in shared_ptr. It is reaponsibility
     * of the tester to make_shared<CryptoModelSignerExpecter> and reset
     * crypto_signer_expecter before the end of test.
     *
     * Usage:
     *   shared_model::crypto::CryptoModelSigner<> crypto_signer(keypair);
     *   EXPECT_CALL(crypto_signer_expecter, sign(block));
     *   crypto_signer.sign(block);
     */

    class CryptoModelSignerExpecter {
     public:
      MOCK_CONST_METHOD1(sign, void(shared_model::interface::Block &));
      MOCK_CONST_METHOD1(sign, void(shared_model::interface::Query &));
      MOCK_CONST_METHOD1(sign, void(shared_model::interface::Transaction &));
    };

    std::shared_ptr<CryptoModelSignerExpecter> crypto_signer_expecter;

    template <>
    template <>
    void CryptoModelSigner<>::sign<shared_model::interface::Block>(
        shared_model::interface::Block &signable) const noexcept {
      crypto_signer_expecter->sign(signable);
    }

    template <>
    template <>
    void CryptoModelSigner<>::sign<shared_model::interface::Query>(
        shared_model::interface::Query &signable) const noexcept {
      crypto_signer_expecter->sign(signable);
    }

    template <>
    template <>
    void CryptoModelSigner<>::sign<shared_model::interface::Transaction>(
        shared_model::interface::Transaction &signable) const noexcept {
      crypto_signer_expecter->sign(signable);
    }

  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_CRYPTO_MODEL_SIGNER_MOCK_HPP
