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

#ifndef AMETSUCHI_WSV_WSV_HPP
#define AMETSUCHI_WSV_WSV_HPP

#include <cstdint>
#include <string>
#include <vector>
namespace iroha {
  namespace ametsuchi {

    namespace wsv {

      class WSV {
       public:
        virtual bool add_account(std::string account_id, uint8_t quorum,
                                 uint32_t status) = 0;
        virtual bool add_peer(const std::string &account_id,
                              const std::string &address, uint32_t state) = 0;
        virtual bool add_signatory(const std::string &account_id,
                                   const std::string &public_key) = 0;

        virtual std::vector<std::string> get_peers(bool committed) = 0;

        virtual void commit_transaction() = 0;

        virtual void commit_block() = 0;

        virtual void rollback_transaction() = 0;

        virtual void rollback_block() = 0;
      };

    }  // namespace wsv

  }  // namespace ametsuchi
}  // namespace iroha
#endif  // AMETSUCHI_WSV_WSV_HPP
