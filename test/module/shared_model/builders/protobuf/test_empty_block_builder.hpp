/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "builders/protobuf/builder_templates/empty_block_template.hpp"
#include "module/shared_model/validators/validators.hpp"

#ifndef IROHA_TEST_EMPTY_BLOCK_BUILDER_HPP
#define IROHA_TEST_EMPTY_BLOCK_BUILDER_HPP

/**
 * Builder alias, to build shared model proto empty block object avoiding
 * validation and "required fields" check
 */
using TestEmptyBlockBuilder = shared_model::proto::TemplateEmptyBlockBuilder<
    (1 << shared_model::proto::TemplateEmptyBlockBuilder<>::total) - 1,
    shared_model::validation::AlwaysValidValidator,
    shared_model::proto::EmptyBlock>;

#endif  // IROHA_TEST_EMPTY_BLOCK_BUILDER_HPP
