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

#ifndef IROHA_WSVRESTORERIMPL_HPP
#define IROHA_WSVRESTORERIMPL_HPP

#include "ametsuchi/wsv_restorer.hpp"
#include "common/result.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Recover WSV (World State View).
     * @return true on success, otherwise false
     */
    class WsvRestorerImpl : public WsvRestorer {
     public:
      virtual ~WsvRestorerImpl() = default;
      /**
       * Recover WSV (World State View).
       * Drop storage and apply blocks one by one.
       * @param storage of blocks in ledger
       * @return void on success, otherwise error string
       */
      virtual expected::Result<void, std::string> restoreWsv(
          Storage &storage) override;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_WSVRESTORERIMPL_HPP
