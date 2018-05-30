/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_EMPTY_BLOCK_BUILDER_HPP
#define IROHA_PROTO_EMPTY_BLOCK_BUILDER_HPP

#include "builders/protobuf/builder_templates/empty_block_template.hpp"

namespace shared_model {
  namespace proto {

    using EmptyBlockBuilder = TemplateEmptyBlockBuilder<>;

    using UnsignedEmptyBlockBuilder = TemplateEmptyBlockBuilder<
        0,
        shared_model::validation::DefaultEmptyBlockValidator,
        shared_model::proto::EmptyBlock>;
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_EMPTY_BLOCK_BUILDER_HPP
