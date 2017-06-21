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

#include <algorithm>
#include <common.hpp>
#include <crypto/base64.hpp>
#include <crypto/hash.hpp>
#include "dao_hash_provider.hpp"

namespace iroha {
  namespace dao {
    class HashProviderImpl : public HashProvider<crypto::ed25519::PUBLEN> {
     public:
      iroha::hash256_t get_hash(const Proposal& proposal) override {
        std::string tx_concat_hash;
        for (auto tx : proposal.transactions) {
          auto tx_hash_arr = get_hash(tx);
          std::string tx_hash_str = crypto::digest_to_hexdigest(
              tx_hash_arr.data(), crypto::ed25519::PUBLEN);

          tx_concat_hash += tx_hash_str;
        }
        std::string tx_concat_hash_hex = crypto::sha3_256_hex(tx_concat_hash);
        auto concat_hash_digest =
            crypto::hexdigest_to_digest(tx_concat_hash_hex);

        iroha::hash256_t res;
        std::copy_n(concat_hash_digest->begin(), res.size(), res.begin());
        return res;
      };

      iroha::hash256_t get_hash(const Block& block) override {
        std::string concat;

        // block height
        concat += std::to_string(block.height);

        // prev_hash
        std::copy(block.prev_hash.begin(), block.prev_hash.end(),
                  std::back_inserter(concat));

        // txnumber
        concat += std::to_string(block.txs_number);

        // merkle root
        std::copy(block.merkle_root.begin(), block.merkle_root.end(),
                  std::back_inserter(concat));

        for (auto tx : block.transactions) {
          auto tx_hash_arr = get_hash(tx);
          std::string tx_hash_str = crypto::digest_to_hexdigest(
              tx_hash_arr.data(), crypto::ed25519::PUBLEN);

          concat += tx_hash_str;
        }
        std::string concat_hash_hex = crypto::sha3_256_hex(concat);
        auto concat_hash_digest = crypto::hexdigest_to_digest(concat_hash_hex);

        iroha::hash256_t res;
        std::copy_n(concat_hash_digest->begin(), res.size(), res.begin());
        return res;
      };

      iroha::hash256_t get_hash(const Transaction& tx) {
        std::string concat_hash_commands;
        for (auto command : tx.commands) {
          command.AppendToString(&concat_hash_commands);
        }
        std::copy(tx.creator.begin(), tx.creator.end(),
                  std::back_inserter(concat_hash_commands));

        std::string concat_hash_hex =
            crypto::sha3_256_hex(concat_hash_commands);
        auto concat_hash_digest = crypto::hexdigest_to_digest(concat_hash_hex);
        iroha::hash256_t res;
        std::copy_n(concat_hash_digest->begin(), res.size(), res.begin());
        return res;
      }
    };
  }
}

#endif  // IROHA_DAO_HASH_PROVIDER_IMPL_HPP
