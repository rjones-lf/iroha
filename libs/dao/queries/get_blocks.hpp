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
#ifndef IROHA_DAO_GET_BLOCKS_HPP
#define IROHA_DAO_GET_BLOCKS_HPP

#include <dao/query.hpp>

namespace iroha {
  namespace dao {

    /**
     * Provide user's intent for adding peer to current network
     */
    struct GetBlocks : public Query {

      /**
       * Id from which fetch the blocks
       */
      uint32_t from;
      /**
       * Id to which fetch the blocks
       */
      uint32_t to;
    };
  }  // namespace dao
}  // namespace iroha

#endif  // IROHA_GET_PEER_HPP
