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

#ifndef IROHA_SEED_HPP
#define IROHA_SEED_HPP

#include "cryptography/blob.hpp"

#include "common/types.hpp"

namespace shared_model {
  namespace crypto {
    /**
     * Class for seed representation.
     */
    class Seed : public Blob {
     public:
      explicit Seed(const std::string &seed);

      /// Old model seed does not have a pretty-looking typedef
      using OldSeedType = iroha::blob_t<32>;

      std::string toString() const override;
    };
  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_SEED_HPP
