/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_EMPTY_BLOCK_HPP
#define IROHA_SHARED_MODEL_EMPTY_BLOCK_HPP

#include "interfaces/iroha_internal/abstract_block.hpp"

namespace shared_model {
  namespace interface {

    class EmptyBlock : public AbstractBlock {
     public:
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Block")
            .append("hash", hash().hex())
            .append("height", std::to_string(height()))
            .append("prevHash", prevHash().hex())
            .append("createdtime", std::to_string(createdTime()))
            .append("signatures")
            .appendAll(signatures(), [](auto &sig) { return sig.toString(); })
            .finalize();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_EMPTY_BLOCK_HPP
