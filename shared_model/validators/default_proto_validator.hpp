/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_DEFAULT_PROTO_VALIDATOR_HPP
#define IROHA_DEFAULT_PROTO_VALIDATOR_HPP

#include "protobuf/proto_transaction_validator.hpp"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace validation {

    using DefaultProtoTransactionValidator =
        SignableModelValidator<ProtoTransactionValidator,
                               const interface::Transaction &,
                               FieldValidator,
                               false>;

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_DEFAULT_PROTO_VALIDATOR_HPP
