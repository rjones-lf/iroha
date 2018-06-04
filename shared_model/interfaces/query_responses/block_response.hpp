/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_BLOCK_RESPONSE_HPP
#define IROHA_SHARED_MODEL_BLOCK_RESPONSE_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "utils/string_builder.hpp"
#include "utils/visitor_apply_for_all.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Provide response with block
     */
    class BlockResponse : public ModelPrimitive<BlockResponse> {
     public:
      /**
       * @return Attached block
       */
      virtual const Block &block() const = 0;

      /**
       * Stringify the data.
       * @return string representation of data.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("BlockResponse")
            .append(block().toString())
            .finalize();
      }

      /**
       * @return true if the data are same.
       */
      bool operator==(const ModelType &rhs) const override {
        return block() == rhs.block();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_BLOCK_RESPONSE_HPP
