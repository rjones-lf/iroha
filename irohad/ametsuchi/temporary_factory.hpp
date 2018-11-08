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

#ifndef IROHA_TEMPORARY_FACTORY_HPP
#define IROHA_TEMPORARY_FACTORY_HPP

#include <memory>
#include "common/result.hpp"

namespace iroha {
  namespace ametsuchi {

    class TemporaryWsv;

    class TemporaryFactory {
     public:
      /**
       * Creates a temporary world state view from the current state.
       * Temporary state will be not committed and will be erased on destructor
       * call.
       * Temporary state might be used for transaction validation.
       * @return Created Result with temporary wsv or string error
       */
      virtual expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
      createTemporaryWsv() = 0;

      /**
       * Prepare state which was accumulated in temporary WSV.
       * After preparation, this state is not visible until commited.
       *
       * @param wsv - state which will be prepared.
       */
      virtual void prepareBlock(std::unique_ptr<TemporaryWsv> wsv) = 0;

      virtual ~TemporaryFactory() = default;
    };

  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_TEMPORARY_FACTORY_HPP
