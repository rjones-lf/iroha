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

#ifndef AMETSUCHI_INDEX_BACKEND_REDIS_HPP
#define AMETSUCHI_INDEX_BACKEND_REDIS_HPP

#include <ametsuchi/index/index.hpp>
#include <cpp_redis/cpp_redis>

namespace iroha {

  namespace ametsuchi {

    namespace index {

      class Redis : public Index {
       public:
        Redis(const std::string &host, std::size_t port);
        ~Redis();
        bool add_blockhash_blockid(std::string block_hash,
                                   uint32_t height) override;
        nonstd::optional<uint64_t> get_blockid_by_blockhash(
            std::string hash) override;
        bool add_txhash_blockid_txid(std::string txhash, uint32_t height,
                                     int txid) override;
        bool add_pubkey_txhash(std::string pubkey, std::string txhash) override;
        nonstd::optional<uint64_t> get_txid_by_txhash(
            std::string txhash) override;
        nonstd::optional<uint64_t> get_blockid_by_txhash(
            std::string txhash) override;
        nonstd::optional<std::vector<std::string>>
        get_txhashes_by_pubkey(std::string pubkey) override;
        nonstd::optional<uint64_t> get_last_blockid() override;
        bool exec_multi() override;
        bool discard_multi() override;

       private:
        cpp_redis::redis_client client_, read_client_;
        std::string host_;
        size_t port_;
        // addition to the tx index without committing
        bool _add_txhash_blockid_txid(std::string txhash, uint32_t height,
                                      int txid);
        bool start_multi();
      };

    }  // namespace index

  }  // namespace ametsuchi
}  // namespace iroha
#endif  // AMETSUCHI_INDEX_BACKEND_REDIS_HPP
