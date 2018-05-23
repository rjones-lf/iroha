/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_VARIANT_HPP
#define IROHA_BLOCK_VARIANT_HPP

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/empty_block.hpp"

namespace shared_model {
  namespace interface {

    /// Block variant with either block or empty block
    using BlockVariantType =
        boost::variant<std::shared_ptr<shared_model::interface::Block>,
                       std::shared_ptr<shared_model::interface::EmptyBlock>>;
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_BLOCK_VARIANT_HPP
