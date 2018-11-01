/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CHAIN_VALIDATOR_HPP
#define IROHA_CHAIN_VALIDATOR_HPP

#include <rxcpp/rx.hpp>

namespace shared_model {
  namespace interface {
    class Block;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {
    class MutableStorage;
  }

  namespace validation {

    /**
     * ChainValidator is interface of chain validation,
     * that is required on commit step of consensus
     */
    class ChainValidator {
     public:
      virtual ~ChainValidator() = default;

      // TODO andrei 16.10.18 IR-1761 Rename methods in validators

      /**
       * Validate method provide chain validation for application it to ledger.
       *
       * Chain validation will validate all signatures of new blocks
       * and related meta information such as previous hash, height and
       * other meta information
       * @param blocks - observable with all blocks, that should be applied
       * atomically
       * @param storage - storage that may be modified during loading
       * @return true if commit is valid, false otherwise
       */
      virtual bool validateChain(
          rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
              blocks,
          ametsuchi::MutableStorage &storage) const = 0;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_CHAIN_VALIDATOR_HPP
