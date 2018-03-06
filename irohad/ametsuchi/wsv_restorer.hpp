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
#ifndef IROHA_WSVRESTORER_HPP
#define IROHA_WSVRESTORER_HPP

#include "common/result.hpp"

namespace iroha {
  namespace ametsuchi {

    class Storage;

    /**
     * Interface for World State View restoring from the storage
     */
    class WsvRestorer {
     public:
      virtual ~WsvRestorer() = default;

      /**
       * Recover WSV (World State View).
       * @param storage storage of blocks in ledger
       * @return void on success, otherwise error string
       */
      virtual expected::Result<void, std::string> restoreWsv(
          Storage &storage) = 0;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_WSVRESTORER_HPP
