/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef IROHA_DAO_HASH_PROVIDER_IMPL_HPP
#define IROHA_DAO_HASH_PROVIDER_IMPL_HPP

#include "dao_hash_provider.hpp"
#include <common.hpp>
#include <crypto/hash.hpp>
#include <crypto/base64.hpp>

namespace iroha {
  namespace dao {
    class HashProviderImpl : public HashProvider<crypto::ed25519::PUBLEN> {
     public:
      iroha::hash256_t get_hash(const Proposal& proposal) override {};
      iroha::hash256_t get_hash(const Block& block) override {};

      iroha::hash256_t get_hash(const Transaction& tx) {
        std::string concat_hash_commands = "";
        for (auto command : tx.commands) {
          std::string command_str = command.SerializeAsString();
          std::string command_hash = crypto::sha3_256_hex(command_str);
          concat_hash_commands += command_hash;
        }
        // + concatenate pubkey
        auto tx_creator_hash = crypto::sha3_256(tx.creator.data(), crypto::ed25519::PUBLEN);
        std::string tx_creator_hash_hex = crypto::digest_to_hexdigest(tx_creator_hash->data(), crypto::ed25519::PUBLEN);
//        free(tx_creator_hash);

        concat_hash_commands += tx_creator_hash_hex;

        std::string concat_hash_hex = crypto::sha3_256_hex(concat_hash_commands);
        auto concat_hash_digest = crypto::hexdigest_to_digest(concat_hash_hex);
        iroha::hash256_t res;
        std::copy_n(concat_hash_digest->begin(), res.size(), res.begin());
        return res;
      }
    };
  }
}

#endif  // IROHA_DAO_HASH_PROVIDER_IMPL_HPP
