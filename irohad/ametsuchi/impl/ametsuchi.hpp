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

#ifndef AMETSUCHI_IMPL_AMETSUCHI_HPP
#define AMETSUCHI_IMPL_AMETSUCHI_HPP

#include <ametsuchi/storage.hpp>
#include <ametsuchi/block_store/block_store.hpp>
#include <ametsuchi/index/index.hpp>
#include <ametsuchi/wsv/wsv.hpp>

namespace iroha {

  namespace ametsuchi {

    namespace impl {

      class Ametsuchi : public ametsuchi::Storage {
       public:
        Ametsuchi();  // TODO add parameters for components
        ~Ametsuchi();
        void insert_block(uint64_t block_id,
                          const std::vector <uint8_t> &blob) override;
        void erase_block(uint64_t block_id) override;
        std::vector <uint8_t> get_block(uint64_t block_id) override;
        void insert_block_index(uint64_t block_id,
                                const std::string &hash) override;
        void insert_tx_index(int tx_id, const std::string &hash,
                             uint64_t block_id) override;
        nonstd::optional <uint64_t> get_block_id_by_block_hash(
            const std::string &hash) override;
        nonstd::optional <uint64_t> get_block_id_by_tx_hash(
            const std::string &hash) override;
        nonstd::optional <uint64_t> get_tx_id(
            const std::string &hash) override;
        void add_account(const std::string &account_id, uint8_t quorum,
                         uint32_t status) override;
        void add_signatory(const std::string &account_id,
                           const std::string &public_key) override;
        void remove_signatory(const std::string &account_id,
                              const std::string &public_key) override;
        void add_domain() override;
        void add_peer(const std::string &account_id, const std::string &address,
                      uint32_t state) override;
        std::vector <std::string> get_peers(bool committed) override;
        void commit_block() override;
        std::vector <std::string> get_tx_hash(const std::string &account_id) override;
        void commit_tx() override;
        void rollback_block() override;
        void rollback_tx() override;
        uint64_t last_block_id_store() override;
        nonstd::optional <uint64_t> last_block_id_index() override;
        uint64_t last_block_id_wsv() override;

       private:
        std::unique_ptr <block_store::BlockStore> block_store_;
        std::unique_ptr <index::Index> index_;
        std::unique_ptr <wsv::WSV> wsv_;
      };

    }  // namespace impl

  }  // namespace ametsuchi

} // namespace iroha
#endif  // AMETSUCHI_IMPL_AMETSUCHI_HPP
