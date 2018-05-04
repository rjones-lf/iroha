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

#ifndef IROHA_QUERY_PAYLOAD_META_HPP
#define IROHA_QUERY_PAYLOAD_META_HPP

#include <string>
#include "common/types.hpp"
#include "model/signature.hpp"

namespace iroha {
  namespace model {
    /**
     * This model represents user intent for reading ledger.
     * Concrete queries should extend this interface.
     */
    struct QueryPayloadMeta {
      /**
       * Account id of transaction creator.
       *
       */
      std::string creator_account_id{};

      /**
       * Creation timestamp
       *
       */
      ts64_t created_ts{};

      /**
       * Query counter
       */
      uint64_t query_counter{};

      virtual ~QueryPayloadMeta() {}
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_QUERY_PAYLOAD_META_HPP
