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

      /**
       * Try to apply the block to the storage.
       *
       * While applying the block it will validate its signature and related
       * meta information such as previous hash, height and other
       * @param block - the block to be applied
       * @param storage - storage to which the block is to be applied
       * @return true if commit is valid and successfully applied, false
       * otherwise
       */
      virtual bool validateAndApply(
          std::shared_ptr<const shared_model::interface::Block> block,
          ametsuchi::MutableStorage &storage) const = 0;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_CHAIN_VALIDATOR_HPP
