/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_ABSTRACT_BLOCK_HPP
#define IROHA_SHARED_MODEL_ABSTRACT_BLOCK_HPP

#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Parent class for Block and EmptyBlock
     */
    class AbstractBlock : public Signable<AbstractBlock> {
     public:
      /**
       * @return block number in the ledger
       */
      virtual types::HeightType height() const = 0;

      /**
       * @return hash of a previous block
       */
      virtual const types::HashType &prevHash() const = 0;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_ABSTRACT_BLOCK_HPP
