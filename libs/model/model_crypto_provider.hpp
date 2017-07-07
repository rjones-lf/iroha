/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef IROHA_MODEL_CRYPTO_PROVIDER_HPP
#define IROHA_MODEL_CRYPTO_PROVIDER_HPP

#include <model/model.hpp>

namespace iroha {
  namespace model {

    /**
     * Crypto provider is an abstract service for making cryptography operations
     * for business logic objects (Model).
     */
    class ModelCryptoProvider {
     public:
      /**
       * Method for signature verification of a transaction.
       * @param tx - transaction for verification
       * @return true if transaction signature is valid, otherwise false
       */
      virtual bool verify(const Transaction &tx) = 0;

      /**
       * Method to sign transaction via own private key.
       * @param tx - transaction without signature
       * @return signed transaction by crypto provider
       */
      virtual Transaction &sign(Transaction &tx) = 0;
    };
  }
}
#endif  // IROHA_MODEL_CRYPTO_PROVIDER_HPP
