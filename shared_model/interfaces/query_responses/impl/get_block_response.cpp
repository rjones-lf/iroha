/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/query_responses/get_block_response.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    std::string GetBlockResponse::toString() const {
      return detail::PrettyStringBuilder()
          .init("GetBlockResponse")
          .append(block().toString())
          .finalize();
    }

    bool GetBlockResponse::operator==(const ModelType &rhs) const {
      return block() == rhs.block();
    }

  }  // namespace interface
}  // namespace shared_model
