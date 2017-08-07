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

#include "model/generators/signature_generator.hpp"

namespace iroha {
  namespace model {
    namespace generators {

      Signature generateSignature(size_t seed) {
        Signature sign;
        // sign.pubkey
        sign.pubkey = generator::random_blob<ed25519::pubkey_t::size()>(seed);
        sign.signature = generator::random_blob<ed25519::sig_t::size()>(
            generator::random_number(0, seed));
        return sign;
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha
