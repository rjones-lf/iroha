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

#include <common/types.hpp>
#include <dao/command.hpp>
#include <dao/signature.hpp>
#include <memory>
#include <vector>
#include <dao/signature.hpp>

namespace iroha {
  namespace dao {

    /**
     * Transaction is a DAO-structure that provides abstraction to bunch of
     * commands with signatures and meta-data.
     * Transaction can be divided to {Header, Meta, Body}.
     */
    struct Transaction {
      /**
       * List of signatories that sign transaction
       * HEADER field
       */
      std::vector<Signature> signatures;

      /**
       * Creation timestamp
       * HEADER field
       */
      ts64_t created_ts;

      /**
       * Public key of a transaction creator.
       * META field
       */
      ed25519::pubkey_t creator;

      /**
       * Number for protecting against replay attack.
       * Number that is stored inside of each account.
       * Used to prevent replay attacks.
       * During a stateful validation look at account and compare numbers
       * if number inside a transaction is less than in account,
       * this transaction is replayed.
       * BODY field
       */
      uint64_t tx_counter;

      /**
       * Bunch of commands attached to transaction
       * shared_ptr is used since Proposal has to be copied
       * BODY field
       */
      std::vector<std::shared_ptr<Command>> commands;
    };
  }
}
#endif  // IROHA_TRANSACTION_HPP
