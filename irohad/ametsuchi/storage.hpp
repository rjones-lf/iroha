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

#ifndef IROHA_STORAGE_H
#define IROHA_STORAGE_H

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <nonstd/optional.hpp>

namespace iroha {

  namespace ametsuchi {
    class Storage {
     public:
      // factory
      static std::unique_ptr<Storage> create();

      // block_store
      virtual void insert_block(uint64_t block_id,
                                const std::vector<uint8_t> &blob) = 0;
      virtual void erase_block(uint64_t block_id) = 0;
      virtual std::vector<uint8_t> get_block(uint64_t block_id) = 0;
      virtual uint64_t last_block_id_store() = 0;

      // index
      virtual void insert_block_index(uint64_t block_id,
                                      const std::string &hash) = 0;
      virtual void insert_tx_index(int tx_id, const std::string &hash,
                                   uint64_t block_id) = 0;
      virtual nonstd::optional<uint64_t> get_block_id_by_block_hash(
          const std::string &hash) = 0;
      // TODO merge into one method?
      virtual nonstd::optional<uint64_t> get_block_id_by_tx_hash(
          const std::string &hash) = 0;
      virtual nonstd::optional<uint64_t> get_tx_id(
          const std::string &hash) = 0;
      virtual nonstd::optional<uint64_t> last_block_id_index() = 0;

      virtual std::vector<std::string> get_tx_hash(
          const std::string &account_id) = 0;

      // wsv
      virtual void add_account(const std::string &account_id, uint8_t quorum,
                               uint32_t status) = 0;
      virtual void add_signatory(const std::string &account_id,
                                 const std::string &public_key) = 0;
      virtual void remove_signatory(const std::string &account_id,
                                    const std::string &public_key) = 0;
      virtual void add_domain() = 0;

      virtual void add_peer(const std::string &account_id,
                            const std::string &address, uint32_t state) = 0;
      virtual std::vector<std::string> get_peers(bool committed) = 0;
      virtual uint64_t last_block_id_wsv() = 0;

      virtual void commit_block() = 0;
      virtual void commit_tx() = 0;
      virtual void rollback_block() = 0;
      virtual void rollback_tx() = 0;
    };
  }  // namespace ametsuchi

}  // namespace iroha
#endif  // IROHA_STORAGE_H
