/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_ABSTRACT_BLOCK_HPP
#define IROHA_SHARED_MODEL_ABSTRACT_BLOCK_HPP

#include <memory>

#include "common/byteutils.hpp"
#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "utils/string_builder.hpp"

#ifndef DISABLE_BACKWARD
#include "model/block.hpp"
#include "model/signature.hpp"
#include "model/transaction.hpp"
#endif

namespace shared_model {
  namespace interface {

#ifndef DISABLE_BACKWARD
    class AbstractBlock : public Signable<AbstractBlock, iroha::model::Block> {
#else
    class AbstractBlock : public Signable<AbstractBlock> {
#endif

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
