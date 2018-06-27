/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_QUERY_RESPONSE_BUILDER_HPP
#define IROHA_BLOCK_QUERY_RESPONSE_BUILDER_HPP

#include "interfaces/query_responses/block_query_response.hpp"

namespace shared_model {
  namespace builder {

    /**
     * Builder to construct block query response object
     * @tparam BuilderImpl
     */
    template <typename BuilderImpl>
    class BlockQueryResponseBuilder {
     public:
      std::shared_ptr<shared_model::interface::BlockQueryResponse> build() {
        return clone(builder_.build());
      }

      BlockQueryResponseBuilder blockResponse(
          shared_model::interface::Block &block) {
        BlockQueryResponseBuilder copy(*this);
        copy.builder_ = this->builder_.blockResponse(block);
        return copy;
      }

      BlockQueryResponseBuilder errorResponse(std::string &message) {
        BlockQueryResponseBuilder copy(*this);
        copy.builder_ = this->builder_.errorResponse(message);
        return copy;
      }

     private:
      BuilderImpl builder_;
    };

  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_BLOCK_QUERY_RESPONSE_BUILDER_HPP
