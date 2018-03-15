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

#ifndef IROHA_CRYPTO_VERIFIER_MOCKS_HPP
#define IROHA_CRYPTO_VERIFIER_MOCKS_HPP

#include <gmock/gmock.h>

#include "cryptography/crypto_provider/crypto_verifier.hpp"

namespace shared_model {
  namespace crypto {

    class MockCryptoVerifier : public CryptoVerifier {
     public:
      MOCK_CONST_METHOD1(verify, bool(const shared_model::interface::Block &));
      MOCK_CONST_METHOD1(verify, bool(const shared_model::interface::Query &));
      MOCK_CONST_METHOD1(verify,
                         bool(const shared_model::interface::Transaction &));
    };

  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_CRYPTO_VERIFIER_MOCKS_HPP
