/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TEST_SIGNATURE_BUILDER_HPP
#define IROHA_TEST_SIGNATURE_BUILDER_HPP

#include "module/shared_model/builders/protobuf/common_objects/proto_signature_builder.hpp"

/**
 * Builder alias, for building shared model proto block object avoiding
 * validation and "required fields" check
 */
using TestSignatureBuilder = shared_model::proto::SignatureBuilder;

#endif  // IROHA_TEST_SIGNATURE_BUILDER_HPP
