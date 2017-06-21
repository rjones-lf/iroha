/*
Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.

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
#include "validator.hpp"
#include <datetime/time.hpp>
#include <dao/dao_hash_provider_impl.hpp>

namespace validaton {
  namespace stateless {
    using Transaction = iroha::protocol::Transaction;

    // ENUM {} scheme api service return TX_Status,
    // Validation Code
    // singature from future
    // header hash, body + meta - string concat and take hash - take byte not string
    // Dygest and 2 byte,


    bool validate(const Transaction& tx) {
      return
        tx.header().created_time() <= iroha::time::now64() && // 過去に作られたTxか // TODO: consider when to ignore transactions for being too old
        tx.header().signature_size( ) != 0                          && // 電子署名は含まれているか
        // TODO: calculate hash
        // TODO: verify the signature for each signature in the header
        tx.body().creator_pubkey().size() == 32;                     // 公開鍵は32byteか ToDo configurable
    }
  };
};