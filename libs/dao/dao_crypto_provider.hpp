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

#ifndef IROHA_DAO_CRYPTO_PROVIDER_HPP
#define IROHA_DAO_CRYPTO_PROVIDER_HPP

#include <dao/dao.hpp>

namespace iroha {
  namespace dao {

    /**
     * Crypto provider is an abstract service for making cryptography operations
     * for business logic objects (DAO).
     */
    class DaoCryptoProvider {
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
      virtual Transaction &sign(Transaction &tx) = 0; // TODO: how do you sign the tx? Where is th key?
    };
  }
}
#endif  // IROHA_DAO_CRYPTO_PROVIDER_HPP
