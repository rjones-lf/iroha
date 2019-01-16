/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TEST_ASSET_BUILDER_HPP
#define IROHA_TEST_ASSET_BUILDER_HPP

#include "module/shared_model/builders/protobuf/common_objects/proto_asset_builder.hpp"

/**
 * Builder alias, to build shared model proto block object avoiding validation
 * and "required fields" check
 */
using TestAccountAssetBuilder = shared_model::proto::AssetBuilder;

#endif  // IROHA_TEST_ASSET_BUILDER_HPP
