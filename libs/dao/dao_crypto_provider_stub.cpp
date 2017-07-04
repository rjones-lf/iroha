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

#include <dao/dao_crypto_provider_stub.hpp>

namespace iroha {
  namespace dao {
    bool DaoCryptoProviderStub::verify(const Transaction &tx) {
      return true;
    }

    Transaction &DaoCryptoProviderStub::sign(Transaction &tx) {
      std::cout << "[\033[32mCryptoProvider\033[0m] sign transaction" << std::endl;
      return tx;
    }
  }// namespace dao
}// namespace iroha