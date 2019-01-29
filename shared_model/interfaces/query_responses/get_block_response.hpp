/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_GET_BLOCK_RESPONSE_HPP
#define IROHA_GET_BLOCK_RESPONSE_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Provide response with the block
     */
    class GetBlockResponse : public ModelPrimitive<GetBlockResponse> {
     public:
      /**
       * @return the fetched block
       */
      virtual const Block &block() const = 0;

      std::string toString() const override;

      bool operator==(const ModelType &rhs) const override;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_GET_BLOCK_RESPONSE_HPP
