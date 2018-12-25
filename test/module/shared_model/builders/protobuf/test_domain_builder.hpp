/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "module/shared_model/builders/protobuf/common_objects/proto_domain_builder.hpp"

#ifndef IROHA_TEST_DOMAIN_BUILDER_HPP
#define IROHA_TEST_DOMAIN_BUILDER_HPP

/**
 * Builder alias, to build shared model proto block object avoiding validation
 * and "required fields" check
 */
using TestDomainBuilder = shared_model::proto::DomainBuilder;

#endif  // IROHA_TEST_DOMAIN_BUILDER_HPP
