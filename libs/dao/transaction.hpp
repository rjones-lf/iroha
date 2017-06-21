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

#ifndef IROHA_TRANSACTION_HPP
#define IROHA_TRANSACTION_HPP
#include <block.pb.h>
#include <commands.pb.h>
#include <common.hpp>
#include <vector>
#include "singature.hpp"

namespace iroha {
  namespace dao {
    struct Transaction {
      // HEADER
      std::vector<Signature> signatures;

      // timestamp
      ts64_t created_ts;

      // number that is stored inside each account.
      // Used to prevent replay attacks.
      // During stateful validation look at account and compare numbers
      // if number inside a transaction is less than in account,
      // this transaction is replayed
      uint64_t tx_counter;

      // META
      // transaction creator
      crypto::ed25519::pubkey_t creator;

      // BODY
      std::vector<iroha::protocol::Command> commands;
      static Transaction create(iroha::protocol::Transaction tx);
    };
  }
}
#endif  // IROHA_TRANSACTION_HPP
