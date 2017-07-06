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

#ifndef AMETSUCHI_INDEX_INDEX_HPP
#define AMETSUCHI_INDEX_INDEX_HPP

#include <cstdint>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>

namespace iroha {

  namespace ametsuchi {

    namespace index {

      class Index {
       public:
        virtual bool add_blockhash_blockid(std::string block_hash,
                                           uint32_t height) = 0;
        virtual nonstd::optional<uint64_t> get_blockid_by_blockhash(
            std::string hash) = 0;
        virtual bool add_txhash_blockid_txid(std::string txhash,
                                             uint32_t height, int txid) = 0;
        virtual bool add_pubkey_txhash(std::string pubkey,
                                       std::string txhash) = 0;
        virtual nonstd::optional<uint64_t> get_txid_by_txhash(
            std::string txhash) = 0;
        virtual nonstd::optional<uint64_t> get_blockid_by_txhash(
            std::string txhash) = 0;
        virtual nonstd::optional<std::vector<std::string>>
        get_txhashes_by_pubkey(std::string pubkey) = 0;
        virtual nonstd::optional<uint64_t> get_last_blockid() = 0;
        virtual bool exec_multi() = 0;
        virtual bool discard_multi() = 0;
      };

    }  // namespace index

  }  // namespace ametsuchi
}  // namespace iroha
#endif  // AMETSUCHI_INDEX_INDEX_HPP
