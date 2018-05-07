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

#ifndef IROHA_TRANSACTION_HPP
#define IROHA_TRANSACTION_HPP

#include <common/types.hpp>
#include <memory>
#include <model/command.hpp>
#include <model/signature.hpp>
#include <string>
#include <vector>

namespace iroha {
  namespace model {

    /**
     * Transaction is a Model-structure that provides abstraction to bunch of
     * commands with signatures and meta-data.
     * Transaction can be divided to {Header, Meta, Body}.
     */
    struct Transaction {
      /**
       * List of signatories that sign transaction
       * HEADER field
       */
      std::vector<Signature> signatures{};

      using SignaturesType = decltype(signatures);

      /**
       * Creation timestamp
       * HEADER field
       */
      ts64_t created_ts{};

      /**
       * Account id of transaction creator.
       * META field
       */
      std::string creator_account_id{};

      /**
       * Bunch of commands attached to transaction
       * shared_ptr is used since Proposal has to be copied
       * BODY field
       */
      std::vector<std::shared_ptr<Command>> commands{};

      using CommandsType = decltype(commands);

      bool operator==(const Transaction &rhs) const;
      bool operator!=(const Transaction &rhs) const;
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_TRANSACTION_HPP
