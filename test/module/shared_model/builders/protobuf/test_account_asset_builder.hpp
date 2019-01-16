/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "module/shared_model/builders/protobuf/common_objects/proto_account_asset_builder.hpp"

#ifndef IROHA_TEST_ACCOUNT_BUILDER_HPP
#define IROHA_TEST_ACCOUNT_BUILDER_HPP

/**
 * Builder alias, to build shared model proto block object avoiding validation
 * and "required fields" check
 */
using TestAccountAssetBuilder = shared_model::proto::AccountAssetBuilder;

#endif  // IROHA_TEST_ACCOUNT_BUILDER_HPP
